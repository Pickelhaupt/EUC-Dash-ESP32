/*
   GPLv3 2020 Jesper Ortlund

   Borrowed some snippets from Alex Goodyear's agoodWatch and a fair bit from
   SimpleWatch example in https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library
   By Lewis He

   BLE code is based on the BLE client example ftom the ESP library

   Wheel data decoding is based on Wheellog by Kevin Cooper
   it would have been a lot of work without that code to reference.

   Requires LVGL 7.5+ as aec drawing in earlier versions is incomplete
   Replace the lvgl library in TTGO_TWatch_Library as the included version is 7.3
*/

#include "twatch_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include <soc/rtc.h>
#include "EUCDash.h"
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

boolean doConnect = false;
boolean connected = false;
boolean doScan = false;
boolean watch_running = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
int ride_mode = 0;
int scandelay = 0;
boolean bleConn;
float wheeldata[16];
struct Wheel_constants wheelconst;
String wheelmodel;
//todo, implement auto detect of wheel brand
String Wheel_brand = "KingSong";


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
    // ttgo->stopLvglTick();
    ttgo->bma->enableStepCountInterrupt(false);
    ttgo->displaySleep();
    displayOff = true;
    lenergy = true;
    if (!connected) { // Only enter sleep mode if there is no wheel connected
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
    lv_disp_trig_activity(NULL);
    setbrightness();
    ttgo->openBL();
    displayOff = false;
    if (connected) {
      screenTimeout = ridingScreenTimeout;
    } else {
      screenTimeout = defaultScreenTimeout;
    }
  }
}

void setbrightness() {
  time_t now;
  struct tm  info;
  char buf[64];
  time(&now);
  localtime_r(&now, &info);
  strftime(buf, sizeof(buf), "%H", &info);
  if (connected) {
    if (info.tm_hour > 19 || info.tm_hour < 6) {
      ttgo->setBrightness(96);
    } else {
      ttgo->setBrightness(255);
    }
  } else {
    if (info.tm_hour > 19 || info.tm_hour < 6) {
      ttgo->setBrightness(32);
    } else {
      ttgo->setBrightness(128);
    }
  }
}

void writeBLE (byte* wBLEbyte, int alength) {
  pRemoteCharacteristic->writeValue(wBLEbyte, alength);
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
    }
  }
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
  return connected;
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

void setup()
{
  Serial.begin (115200);

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
  //ttgo->power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
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

  //Clear lvgl counter
  lv_disp_trig_activity(NULL);

  //When the initialization is complete, turn on the backlight
  setbrightness();
  ttgo->openBL();

  //Execute watch only GUI interface
  //if (!connected) {
  //  setup_timeGui();
  //}

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
  pBLEScan->start(2, false);
}

void loop()
{
  bool  rlst;
  uint8_t data;
  //static uint32_t start = 0;
  if (!connected) {
    if (scandelay > 1000) { //Scan for BLE devices around every 5 seconds when not connected
      doScan = true;
    }
    scandelay++;
    //Serial.print("scandelay: "); Serial.println(scandelay);
    if (!watch_running) {
      stop_dash_task();
      setup_timeGui();
      watch_running = true;
      //Serial.println(watch_running);
    }
  }
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
        Serial.println("initialising KingSong");
        //setCpuFrequencyMhz (CPU_FREQ_MEDIUM);
        initks();
        stop_time_task();
        watch_running = false;
        screenTimeout = ridingScreenTimeout;
        setup_LVGui();
    } else {
      Serial.println("We have failed to connect to the server;");
    }
    doConnect = false;
    //    Send necessary initialisation packages to start BLE notifications, do not know why this is needed though
  }
  if (!connected && doScan && (ttgo->bl->isOn())) {
    Serial.println("Disconnected... starting scan");
    BLEDevice::getScan()->start(2);
    scandelay = 0;
    doScan = false;
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
          } else if (screenTimeout == ridingScreenTimeout) {
            if (wheeldata[1] > 1.5) {
              ks_ble_request(0x9B);
              //  } else {
              //    add lights off here
              //  }
            }
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
    lv_task_handler(); //Since this function will loop, it's necessary to manage lv tasks
  } else {
    low_energy();
  }
}
