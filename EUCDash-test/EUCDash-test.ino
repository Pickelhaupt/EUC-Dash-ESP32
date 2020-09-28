/*
   GPLv3 Jesper Ortlund

   Borrowed some snippets from Alex Goodyear's agoodWatch and a fair bit from
   SimpleWatch example in https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library
   By Lewis He
   
   BLE code is based on the BLE client example ftom the ESP library

   Wheel data decoding is based on Wheellog by Kevin Cooper, Cedric Hauber and Palachzzz,
   would have been a lot of work without that code to reference.

   Requires LVGL 7.5+ as aec drawing in earlier versions is incomplete
   Replace the lvgl library in TTGO_TWatch_Library as the included version is 7.3
*/

#include "twatch_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include <soc/rtc.h>
#include "EUCDash_test.h"
#include "BLEDevice.h"

#define G_EVENT_VBUS_PLUGIN         _BV(0)
#define G_EVENT_VBUS_REMOVE         _BV(1)
#define G_EVENT_CHARGE_DONE         _BV(2)

enum {
  Q_EVENT_WIFI_SCAN_DONE,
  Q_EVENT_WIFI_CONNECT,
  Q_EVENT_BMA_INT,
  Q_EVENT_AXP_INT,
} ;

#define WATCH_FLAG_SLEEP_MODE   _BV(1)
#define WATCH_FLAG_SLEEP_EXIT   _BV(2)
#define WATCH_FLAG_BMA_IRQ      _BV(3)
#define WATCH_FLAG_AXP_IRQ      _BV(4)

QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
EventGroupHandle_t isr_group = NULL;
bool lenergy = false;
bool displayOff = false;
TTGOClass *ttgo;
lv_icon_battery_t batState = LV_ICON_CALCULATION;

unsigned int defaultCpuFrequency = CPU_FREQ_NORM;
unsigned int defaultScreenTimeout = DEFAULT_SCREEN_TIMEOUT;
unsigned int ridingScreenTimeout = RIDING_SCREEN_TIMEOUT;
unsigned int screenTimeout = DEFAULT_SCREEN_TIMEOUT;

//temporary model strings, Todo: implement automated id
String wheelmodel = "KS14SMD";
//String wheelmodel = "KS16S";

boolean doConnect = false;
boolean connected = false;
boolean doScan = false;
boolean watch_running = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
float wheeldata[16];
int ride_mode = 0;
int scandelay = 0;
boolean bleConn;
byte KS_BLEreq[20];


/************************************************
   For Kingsong and Gotway EUCs
   Inmotion/Solowheel and Ninebot/Segway use
   different UUIDs, I do not have any of those
   wheels so cannot test, Might need a different
   init procedure as well. Edit this if you want
   to support additional manufacturers. It would
   be possible to automate this as well.
 *************************************************/
/*
   For some reason the KS wheels announce 2 UUIDs, the first one
   "FFF0" does not contain the data we are looking for
*/
static BLEUUID serviceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
//The Service UUID that contains the useful notifications
static BLEUUID serviceUUID2("0000ffe0-0000-1000-8000-00805f9b34fb");
// The characteristic UUID of all wheel notifications
static BLEUUID charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");



/******************************************************************
   Function managing the entry and exit from sleep/power save mode
 ******************************************************************/
void low_energy()
{
  if (ttgo->bl->isOn()) { //Go to sleep / switch off display
    log_i("low_energy() - BL is on");
    xEventGroupSetBits(isr_group, WATCH_FLAG_SLEEP_MODE);

    ttgo->closeBL();
    ttgo->stopLvglTick();
    ttgo->bma->enableStepCountInterrupt(false);
    ttgo->displaySleep();
    displayOff = true;
    if (!connected) { // Only enter sleep mode if there is no wheel connected
      lenergy = true;
      setCpuFrequencyMhz (CPU_FREQ_MIN);
      gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
      gpio_wakeup_enable ((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
      esp_sleep_enable_gpio_wakeup ();
      esp_light_sleep_start();
    }
  } else { //Wake from sleep mode
    Serial.println("waking up");
    log_i("low_energy() - BL is off");
    ttgo->startLvglTick();
    ttgo->displayWakeup();
    ttgo->rtc->syncToSystem();
    // updateBatteryLevel();
    // updateBatteryIcon(batState);
    //updateTime();
    lv_disp_trig_activity(NULL);
    ttgo->openBL();
    displayOff = true;
    if (connected) {
      screenTimeout = ridingScreenTimeout;
    } else {
      screenTimeout = defaultScreenTimeout;
    }
  }
}

/*******************************************************************
   BLE Callback notify, will loop here when wheel is connected
   Decoding is only done if the data format is correct
   This might need to be changed if adapted to other manufacturers
 ******************************************************************/
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  //Only decode if package contains relevant data
  if (length == 20) {
    if (pData[0] == 0xAA && pData[1] == 0x55) {
      decodeKS(pData); // For Kingsong only atm.
      //Invoke the lvgl task handler from within the notifycallback handler
      //(updates the text labels everytime there is a valid notification from the characteristic FFE1)
      lv_task_handler(); //Since this function will loop, it's necessary to manage lv tasks
    }
  }
}

/*************************************************************
    Kingsong wheel data decoder adds current values to
    the wheeldata array. Prootocol decoding from Wheellog by
    Kevin Cooper, Cedric Hauber and Palachzzz,
    would have been a lot of work without that code
    to reference.
    Should be fairly easy to adapt this to Gotway as well.
    Function is called on by the notifyCallback function
    Todo:
    - Add Serial and model number
    - Add periodical polling of speed settings? Verify by
      testing with low battery
    - Add battery calc for more voltages 82, 100 etc.
 ************************************************************/
static void decodeKS (byte KSdata[]) {

  int rMode;
  float Battpct;

  //Parse incoming BLE Notifications
  if (KSdata[16] == 0xa9) { // Data package type 1 voltage/speed/odo/current/temperature
    int rBattvolt = decode2byte(KSdata[2], KSdata[3]);
    int rSpeed = decode2byte(KSdata[4], KSdata[5]);
    int rOdo = decode4byte(KSdata[6], KSdata[7], KSdata[8], KSdata[9]);
    int rCurr = decode2byte(KSdata[10], KSdata[11]);
    int rTemp = decode2byte(KSdata[12], KSdata[13]);
    if (KSdata[15] == 0xe0) {
      int rMode = KSdata[14];
    }
    // Convert to readable numbers and add to the wheeldata array
    wheeldata[0] = rBattvolt / 100.00; //Battry voltage
    wheeldata[1] = rSpeed / 100.00;    //Current speed
    wheeldata[2] = rOdo / 1000.00;     //Total kms ridden
    if (rCurr > 32767) { // Ugly hack to display negative currents, must be a better way
      rCurr = rCurr - 65536;
    }
    wheeldata[3] = rCurr / 100.00;    //Current Current :)
    wheeldata[4] = rTemp / 100.00;    //Current wheel temperature
    wheeldata[5] = rMode;
    // Battery percentage, using betterPercents algorithm from Wheellog, only for 67V atm
    if (rBattvolt > 6680) {
      Battpct = 100;
    } else if (rBattvolt > 5440) {
      Battpct = (rBattvolt - 5320) / 13.6;
    } else if (rBattvolt > 5120) {
      Battpct = (rBattvolt - 5120) / 36.0;
    } else {
      Battpct = 0;
    }
    wheeldata[6] = Battpct;
    wheeldata[7] = wheeldata[0] * wheeldata[3]; //current power usage
  }
  else if (KSdata[16] == 0xb9) { // Data package type 3 distance/time/top speed/fan
    int rDistance = decode4byte(KSdata[2], KSdata[3], KSdata[4], KSdata[5]); // trip counter resets when wheel off
    int rWheeltime = decode2byte(KSdata[6], KSdata[7]); //time since turned on
    int rTopspeed = decode2byte(KSdata[8], KSdata[9]); //Max speed since last power on
    int rFanstate = KSdata[12];  //1 if fan is running

    wheeldata[8] = rDistance / 1000.0; //convert from m to km
    wheeldata[9] = rWheeltime;
    wheeldata[10] = rTopspeed / 100.0;
    wheeldata[11] = rFanstate;
  }
  else if (KSdata[16] == 0xb5) {
    wheeldata[12] = KSdata[4]; //Alarmspeed 1
    wheeldata[13] = KSdata[6]; //Alarmspeed 2
    wheeldata[14] = KSdata[8]; //Alarmspeed 3
    wheeldata[15] = KSdata[10]; //Max speed (tiltback)
  }

  //Debug -- testing, print all data to serial
  Serial.print(wheeldata[0]); Serial.println(" V");
  Serial.print(wheeldata[1]); Serial.println(" kmh");
  Serial.print(wheeldata[2]); Serial.println(" km");
  Serial.print(wheeldata[3]); Serial.println(" A");
  Serial.print(wheeldata[4]); Serial.println(" C");
  Serial.print(wheeldata[5]); Serial.println(" rmode");
  Serial.print(wheeldata[6]); Serial.println(" %");
  Serial.print(wheeldata[7]); Serial.println(" W");
  Serial.print(wheeldata[8]); Serial.println(" km");
  Serial.print(wheeldata[9]); Serial.println(" time");
  Serial.print(wheeldata[10]); Serial.println(" kmh");
  Serial.print(wheeldata[11]); Serial.println(" fan");
  Serial.print(wheeldata[12]); Serial.println(" alarm1");
  Serial.print(wheeldata[13]); Serial.println(" alarm2");
  Serial.print(wheeldata[14]); Serial.println(" alarm3");
  Serial.print(wheeldata[15]); Serial.println(" maxspeed");

  
} // End decodeKS

static int decode2byte(byte byte1, byte byte2) { //converts big endian 2 byte value to int
  int val;
  val = (byte1 & 0xFF) + (byte2 << 8);
  return val;
}

static int decode4byte(byte byte1, byte byte2, byte byte3, byte byte4) { //converts bizarre 4 byte value to int
  int val;
  val = (byte1 << 16) + (byte2 << 24) + byte3 + (byte4 << 8);
  return val;
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }
    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID2);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found our characteristic");
  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
  }

  // Register for notify
  if (pRemoteCharacteristic->canNotify() == true) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  connected = true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks

void ks_ble_request(byte reqtype) {
  /****************************************
   * reqtype is the byte representing the request id
   * 0x9B -- Serial Number
   * 0x63 -- Manufacturer and model
   * 0x98 -- speed alarm settings and tiltback (Max) speed
   * Responses to the request is handled by the notification handler
   * and will be added to the wheeldata[] array
   */
  byte KS_BLEreq[20] = {0x00}; //set array to zero
  KS_BLEreq[0] = 0xAA;  //Header byte 1
  KS_BLEreq[1] = 0x55;  //Header byte 2
  KS_BLEreq[16] = reqtype;  // This is the byte that specifies what data is requested
  KS_BLEreq[17] = 0x14; //Last 3 bytes also needed
  KS_BLEreq[18] = 0x5A;
  KS_BLEreq[19] = 0x5A;
  pRemoteCharacteristic->writeValue(KS_BLEreq, 20);
}

void initks() {
 /*
  *   Request Kingsong Model Name, serial number and speed settings
  *   This must be done before any BLE notifications will be pused by the KS wheel
  */
  Serial.println("requesting model..");
  ks_ble_request(0x9B);
  delay(200);
  Serial.println("requesting serial..");
  ks_ble_request(0x63);
  delay(200);
  Serial.println("requesting speed settings..");
  ks_ble_request(0x98);
  delay(200);
} //End of initks

void setup()
{
  Serial.begin (115200);

  //Temporary setting of some model specific parametes, 
  //Todo: automatic model identification
  if (wheelmodel = "KS14SMD") {
    maxcurrent = 30;
    crittemp = 65;
    warntemp = 50;
  } else if (wheelmodel = "KS16S") {
    maxcurrent = 35;
    crittemp = 65;
    warntemp = 50;
  } else {
    maxcurrent = 30;
    crittemp = 65;
    warntemp = 50;
  }

  //Create a program that allows the required message objects and group flags
  g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
  g_event_group = xEventGroupCreate();
  isr_group = xEventGroupCreate();

  ttgo = TTGOClass::getWatch();

  //Initialize TWatch
  ttgo->begin();

  // Turn on the IRQ used
  ttgo->power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1, AXP202_ON);
  ttgo->power->enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_FINISHED_IRQ, AXP202_ON);
  ttgo->power->clearIRQ();

  //Turn off unused power
  ttgo->power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
  ttgo->power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
  ttgo->power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);
  ttgo->power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

  //Initialize lvgl
  ttgo->lvgl_begin();
  ttgo->motor_begin();

  //Initialize bma423
  ttgo->bma->begin();

  //Enable BMA423 interrupt
  ttgo->bma->attachInterrupt();

  //Connection interrupted to the specified pin
  pinMode(BMA423_INT1, INPUT);
  attachInterrupt(BMA423_INT1, [] {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    EventBits_t  bits = xEventGroupGetBitsFromISR(isr_group);
    if (bits & WATCH_FLAG_SLEEP_MODE)
    {
      // Use an XEvent when waking from low energy sleep mode.
      xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_BMA_IRQ, &xHigherPriorityTaskWoken);
    } else
    {
      // Use the XQueue mechanism when we are already awake.
      uint8_t data = Q_EVENT_BMA_INT;
      xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR ();
    }
  }, RISING);

  // Connection interrupted to the specified pin
  pinMode(AXP202_INT, INPUT);
  attachInterrupt(AXP202_INT, [] {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    EventBits_t  bits = xEventGroupGetBitsFromISR(isr_group);
    if (bits & WATCH_FLAG_SLEEP_MODE)
    {
      // Use an XEvent when waking from low energy sleep mode.
      xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_AXP_IRQ, &xHigherPriorityTaskWoken);
    } else
    {
      // Use the XQueue mechanism when we are already awake.
      uint8_t data = Q_EVENT_AXP_INT;
      xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
    }
    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR ();
    }
  }, FALLING);

  //Check if the RTC clock matches, if not, use compile time
  ttgo->rtc->check();

  //Synchronize time to system time
  ttgo->rtc->syncToSystem();

  //Execute your own GUI interface
  if (!connected) {
    setup_timeGui();
  }
  //Clear lvgl counter
  lv_disp_trig_activity(NULL);

  //When the initialization is complete, turn on the backlight
  ttgo->openBL();

  /*
     Setup the axes for the TWatch-2020-V1 for the tilt detection.
  */
  struct bma423_axes_remap remap_data;

  remap_data.x_axis = 0;
  remap_data.x_axis_sign = 1;
  remap_data.y_axis = 1;
  remap_data.y_axis_sign = 0;
  remap_data.z_axis  = 2;
  remap_data.z_axis_sign  = 1;

  ttgo->bma->set_remap_axes(&remap_data);

  /*
     Enable the double tap wakeup.
  */
  ttgo->bma->enableWakeupInterrupt (true);

  /*****************
     Setup BLE
  ********************/
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop()
{
  bool  rlst;
  uint8_t data;
  static uint32_t start = 0;
  if (!connected) {
    if (scandelay > 1000) { //Scan for BLE devices around every 5 seconds when nor connected
      doScan = true;
    }
    scandelay++;
    Serial.print("scandelay: "); Serial.println(scandelay);
    if (!watch_running){
        stop_dash_task();
        setup_timeGui();
        watch_running = true;
        Serial.println(watch_running);
      }
  }


  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server;");
      
    }
    doConnect = false;

    //    Send necessary initialisation packages to start BLE notifications, do not know why this is needed though
    Serial.println("initialising KingSong");
    initks();
    stop_time_task();
    watch_running = false;
    setup_LVGui();
  }
  if (connected) {
    ride_mode = 1;
  }
  else if (doScan && (ttgo->bl->isOn())) {
    ride_mode = 0;
    Serial.println("Disconnected... starting scan");
    ride_mode = 0;
    BLEDevice::getScan()->start(2);
    scandelay = 0;
    doScan = false;
  } else {
    ride_mode = 0;
    Serial.println("Disconnected... scanning is off");
  }

  // An XEvent signifies that there has been a wakeup interrupt, bring the CPU out of low energy mode
  EventBits_t  bits = xEventGroupGetBits(isr_group);
  if (bits & WATCH_FLAG_SLEEP_EXIT) {
    if (lenergy) {
      lenergy = false;
      setCpuFrequencyMhz (defaultCpuFrequency);
    }
    low_energy();

    if (bits & WATCH_FLAG_BMA_IRQ) {
      log_i("WATCH_FLAG_BMA_IRQ");
      do {
        rlst =  ttgo->bma->readInterrupt();
      } while (!rlst);
      xEventGroupClearBits(isr_group, WATCH_FLAG_BMA_IRQ);
    }
    if (bits & WATCH_FLAG_AXP_IRQ) {
      log_i("WATCH_FLAG_AXP_IRQ");
      ttgo->power->readIRQ();
      ttgo->power->clearIRQ();
      xEventGroupClearBits(isr_group, WATCH_FLAG_AXP_IRQ);
    }
    xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_EXIT);
    xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_MODE);
  }
  if ((bits & WATCH_FLAG_SLEEP_MODE)) {
    //! No event processing after entering the information screen
    return;
  }

  //! Normal polling
  if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
    switch (data) {
      case Q_EVENT_BMA_INT:
        log_i("Q_EVENT_BMA_IRQ");

        do {
          rlst =  ttgo->bma->readInterrupt();
        } while (!rlst);

        if (ttgo->bma->isDoubleClick()) {
          if (screenTimeout == defaultScreenTimeout)
          {
            screenTimeout--;
            ttgo->setBrightness(255);
          }
          else
          {
            // screenTimeout = 5 * 60 * 1000;
            // torchOn();
            // setCpuFrequencyMhz (CPU_FREQ_MIN);
          }
        }
        break;

      case Q_EVENT_AXP_INT:
        log_i("Q_EVENT_AXP_INT");

        ttgo->power->readIRQ();
        if (ttgo->power->isVbusPlugInIRQ()) {
          batState = LV_ICON_CHARGE;
          // updateBatteryIcon(LV_ICON_CHARGE);
        }
        if (ttgo->power->isVbusRemoveIRQ()) {
          batState = LV_ICON_CALCULATION;
          // updateBatteryIcon(LV_ICON_CALCULATION);
        }
        if (ttgo->power->isChargingDoneIRQ()) {
          batState = LV_ICON_CALCULATION;
          // updateBatteryIcon(LV_ICON_CALCULATION);
        }
        if (ttgo->power->isPEKShortPressIRQ()) {
          ttgo->power->clearIRQ();
          low_energy();
          return;
        }
        ttgo->power->clearIRQ();
        break;
    }
  }
  if (lv_disp_get_inactive_time(NULL) < screenTimeout) {
    lv_task_handler();
  } else {
    low_energy();
  }
}
