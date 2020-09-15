/**
 * A dashboard for electric unicycles, currently only works with KingSong 67 volt wheels.
 * It can probably be adapted to work with multiple wheels fairly easily. 
 * only tested on TTGO T-Watch 2020 so far. Uses LVGL for the interface so should run on other
 * ESP32 implementations with some modifications.
 * TODO, Add support for more wheels, implement power saving features, improve interface
 * Working first draft v0.1 2020-09-15
 */

#include "BLEDevice.h"
//#include "BLEScan.h"
#define LILYGO_WATCH_2020_V1
#define LILYGO_WATCH_LVGL
#include <LilyGoWatch.h>
TTGOClass *ttgo;
TFT_eSPI *tft;
PCF8563_Class *rtc;

int value = 1;
// The remote service we wish to connect to.
static BLEUUID serviceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUID2("0000ffe0-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
float wheeldata[16];
String wheelmodel = "unknown";

//Static LV objects
lv_obj_t * volt; //wheeldata[0]
lv_obj_t * voltUnit;
lv_obj_t * spd; //wheeldata[1]
lv_obj_t * spdUnit;
lv_obj_t * odo; //wheeldata[2]
lv_obj_t * odoUnit;
lv_obj_t * odoLabel;
lv_obj_t * amp; //wheeldata[3]
lv_obj_t * ampUnit;
lv_obj_t * temp; //wheeldata[4]
lv_obj_t * tempUnit;
lv_obj_t * ridingmode; //wheeldata[5]
lv_obj_t * battpercent; //wheeldata[6]
lv_obj_t * battpercentUnit; 
lv_obj_t * power; //wheeldata[7];
lv_obj_t * powerUnit;
lv_obj_t * trip; //wheeldata[8]
lv_obj_t * tripUnit;
lv_obj_t * tripLabel;
lv_obj_t * wtime; //wheeldata[9]
lv_obj_t * wtimeUnit;
lv_obj_t * topspeed; //wheeldata[10]
lv_obj_t * topspeedUnit;
lv_obj_t * topspeedLabel;
lv_obj_t * fanstate; //wheeldata[11]
lv_obj_t * alarm3;
lv_obj_t * alarm3Label;
lv_obj_t * maxspeed;
lv_obj_t * maxspeedLabel;

//Static LV styles
static lv_style_t spdStyle;
static lv_style_t spdStyley;
static lv_style_t spdStyler;
static lv_style_t mediumStyle;
static lv_style_t mediumStyley;
static lv_style_t mediumStyler;
static lv_style_t unit;
static lv_style_t small;
static lv_style_t smallm;
static lv_style_t sm;
static lv_style_t smm;

//Static lv subs
static void label_update(lv_task_t * t);


static void lv_labels_create() {
//Define Styles Font and colour of the labels
//Central speed display (extra large font)
    lv_style_init(&spdStyle);
    lv_style_set_text_color(&spdStyle, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_style_set_text_font(&spdStyle, LV_STATE_DEFAULT, &lv_font_montserrat_48);
    lv_style_init(&spdStyley);
    lv_style_set_text_color(&spdStyley, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_font(&spdStyley, LV_STATE_DEFAULT, &lv_font_montserrat_48);
    lv_style_init(&spdStyler);
    lv_style_set_text_color(&spdStyler, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_font(&spdStyler, LV_STATE_DEFAULT, &lv_font_montserrat_48);
//Medium
    lv_style_init(&mediumStyle);
    lv_style_set_text_color(&mediumStyle, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_style_set_text_font(&mediumStyle, LV_STATE_DEFAULT, &lv_font_montserrat_38);
    lv_style_init(&mediumStyley);
    lv_style_set_text_color(&mediumStyley, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_font(&mediumStyley, LV_STATE_DEFAULT, &lv_font_montserrat_38);
    lv_style_init(&mediumStyler);
    lv_style_set_text_color(&mediumStyler, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_font(&mediumStyler, LV_STATE_DEFAULT, &lv_font_montserrat_38);
//Small medium
    lv_style_init(&sm);
    lv_style_set_text_color(&sm, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_style_set_text_font(&sm, LV_STATE_DEFAULT, &lv_font_montserrat_26);
    lv_style_init(&smm);
    lv_style_set_text_color(&smm, LV_STATE_DEFAULT, LV_COLOR_MAGENTA);
    lv_style_set_text_font(&smm, LV_STATE_DEFAULT, &lv_font_montserrat_26);

//Small font
    lv_style_init(&small);
    lv_style_set_text_color(&small, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_style_set_text_font(&small, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    lv_style_init(&smallm);
    lv_style_set_text_color(&smallm, LV_STATE_DEFAULT, LV_COLOR_MAGENTA);
    lv_style_set_text_font(&smallm, LV_STATE_DEFAULT, &lv_font_montserrat_16);

//Units, (small font to conserve screen real estate).
    lv_style_init(&unit);
    lv_style_set_text_color(&unit, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_style_set_text_font(&unit, LV_STATE_DEFAULT, &lv_font_montserrat_18);

//Create text Labels for all data to shown on the display
// Voltage
    volt = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(volt, LV_OBJ_PART_MAIN, &mediumStyle);
    lv_obj_align(volt, NULL, LV_ALIGN_CENTER, -65, -75);

    voltUnit = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(voltUnit, LV_OBJ_PART_MAIN, &unit);
    lv_obj_align(voltUnit, NULL, LV_ALIGN_CENTER, -5, -69);
//Current
    amp = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(amp, LV_OBJ_PART_MAIN, &mediumStyle);
    lv_obj_align(amp, NULL, LV_ALIGN_CENTER, 60, -75);

    ampUnit = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(ampUnit, LV_OBJ_PART_MAIN, &unit);
    lv_obj_align(ampUnit, NULL, LV_ALIGN_CENTER, 110, -69);
//Speed    
    spd = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(spd, LV_OBJ_PART_MAIN, &spdStyle);
    lv_obj_align(spd, NULL, LV_ALIGN_CENTER, 0, 0);
    
    spdUnit = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(spdUnit, LV_OBJ_PART_MAIN, &unit);
    lv_obj_align(spdUnit, NULL, LV_ALIGN_CENTER, 0, 30);
//Topspeed
    topspeed = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(topspeed, LV_OBJ_PART_MAIN, &sm);
    lv_obj_align(topspeed, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 5);

    topspeedUnit = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(topspeedUnit, LV_OBJ_PART_MAIN, &small);
    lv_obj_align(topspeedUnit, NULL, LV_ALIGN_IN_RIGHT_MID, -15, 24);

    topspeedLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(topspeedLabel, LV_OBJ_PART_MAIN, &small);
    lv_obj_align(topspeedLabel, NULL, LV_ALIGN_IN_RIGHT_MID, -15, -15);

//Trip
    trip = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(trip, LV_OBJ_PART_MAIN, &sm);
    lv_obj_align(trip, NULL, LV_ALIGN_IN_LEFT_MID, 10, 95);

   // tripUnit = lv_label_create(lv_scr_act(), NULL);
   // lv_obj_add_style(tripUnit, LV_OBJ_PART_MAIN, &small);
   // lv_obj_align(tripUnit, NULL, LV_ALIGN_CENTER, -30, 98);

    tripLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(tripLabel, LV_OBJ_PART_MAIN, &small);
    lv_obj_align(tripLabel, NULL, LV_ALIGN_IN_LEFT_MID, 10, 75);

//Odo
    odo = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(odo, LV_OBJ_PART_MAIN, &sm);
    lv_label_set_align(odo, LV_LABEL_ALIGN_RIGHT);
    lv_obj_align(odo, NULL, LV_ALIGN_IN_RIGHT_MID, -26, 95);

   // odoUnit = lv_label_create(lv_scr_act(), NULL);
   // lv_obj_add_style(odoUnit, LV_OBJ_PART_MAIN, &small);
    //lv_obj_align(odoUnit, NULL, LV_ALIGN_CENTER, 98, 98);

    odoLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(odoLabel, LV_OBJ_PART_MAIN, &small);
    lv_label_set_align(odoLabel, LV_LABEL_ALIGN_RIGHT);
    lv_obj_align(odoLabel, NULL, LV_ALIGN_IN_RIGHT_MID, -45, 75);
    
//Maxspeed
    maxspeed = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(maxspeed, LV_OBJ_PART_MAIN, &smm);
    lv_obj_align(maxspeed, NULL, LV_ALIGN_IN_LEFT_MID, 10, 27);

    maxspeedLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(maxspeedLabel, LV_OBJ_PART_MAIN, &smallm);
    lv_obj_align(maxspeedLabel, NULL, LV_ALIGN_IN_LEFT_MID, 10, 10);
    
//Alarm3speed
    alarm3 = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(alarm3, LV_OBJ_PART_MAIN, &smm);
    lv_obj_align(alarm3, NULL, LV_ALIGN_IN_LEFT_MID, 10, -10);

    alarm3Label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(alarm3Label, LV_OBJ_PART_MAIN, &smallm);
    //lv_obj_align(alarm3Label, NULL, LV_ALIGN_CENTER, -80, -28);
    lv_obj_align(alarm3Label, NULL, LV_ALIGN_IN_LEFT_MID, 10, -28);
    
//Create task -- update freq 4/s
    lv_task_t * t = lv_task_create(label_update, 250, LV_TASK_PRIO_MID, NULL);
    lv_task_ready(t);
}

//Update label data from the wheeldata array, could not figure out how to print floats directly
//and had to resort to ugly conversion hack
static void label_update(lv_task_t * t) {
  
//Update voltage
  char voltstring[4];
  dtostrf(wheeldata[0], 4, 1, voltstring);
  if (wheeldata[0] < 60) { // Turn voltage display yellow when 50%remain
    lv_obj_add_style(volt, LV_OBJ_PART_MAIN, &mediumStyley);
  } else if (wheeldata[0] < 55) { // Turn voltage display red when 10% remain
    lv_obj_add_style(volt, LV_OBJ_PART_MAIN, &mediumStyler);
  } else {
    lv_obj_add_style(volt, LV_OBJ_PART_MAIN, &mediumStyle);
  }
  lv_label_set_text(volt, voltstring);
  lv_label_set_text(voltUnit, "V");
  
//Update speed
  char speedstring[4];
  dtostrf(wheeldata[1], 4, 1, speedstring);
  if (wheeldata[1] >= wheeldata[15]){ //Turn speed display red when tiltback speed is reached
    lv_obj_add_style(spd, LV_OBJ_PART_MAIN, &spdStyler);
  }
  if (wheeldata[1] >= wheeldata[14]){ //Turn speed display yellow when 3rd alarm is reached
    lv_obj_add_style(spd, LV_OBJ_PART_MAIN, &spdStyley);
  } else {
    lv_obj_add_style(spd, LV_OBJ_PART_MAIN, &spdStyle);
  }
  lv_label_set_text(spd, speedstring);
  lv_label_set_text(spdUnit, "kmh");
  
//Update current
  char ampstring[4];
  dtostrf(wheeldata[3], 4, 1, ampstring);
  lv_label_set_text(amp, ampstring);
  lv_label_set_text(ampUnit, "A");
  
//Update top speed reached
  char topspeedstring[4];
  dtostrf(wheeldata[10], 4, 1, topspeedstring);
  lv_label_set_text(topspeed, topspeedstring);
  lv_label_set_text(topspeedUnit, "kmh");
  lv_label_set_text(topspeedLabel, "max");
  
//Update max speed (tiltback)
  char maxspeedstring[2];
  dtostrf(wheeldata[15], 2, 0, maxspeedstring);
  lv_label_set_text(maxspeed, maxspeedstring);
  lv_label_set_text(maxspeedLabel, "tilt");
  
//Update 3:rd alarm speed
  char alarm3string[2];
  dtostrf(wheeldata[14], 2, 0, alarm3string);
  lv_label_set_text(alarm3, alarm3string);
  lv_label_set_text(alarm3Label, "alm3");
  
//Update trip meter
  char tripstring[6];
  dtostrf(wheeldata[8], 4, 2, tripstring);
  lv_label_set_text(trip, tripstring);
  //lv_label_set_text(tripUnit, "km");
  lv_label_set_text(tripLabel, "Trip (km)");
  
//Update odometer
  char odostring[6];
  dtostrf(wheeldata[2], 4, 2, odostring);
  lv_label_set_text(odo, odostring);
  //lv_label_set_text(odoUnit, "km");
  lv_label_set_text(odoLabel, "Odo (km)");
}

lv_obj_t *setupGUI() //add black background -- fix this
{
    static lv_style_t cont_style;
    lv_style_init(&cont_style);
    lv_style_set_radius(&cont_style, LV_OBJ_PART_MAIN, 0);
    lv_style_set_bg_color(&cont_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&cont_style, LV_OBJ_PART_MAIN, LV_OPA_COVER);
    lv_style_set_border_width(&cont_style, LV_OBJ_PART_MAIN, 0);

    lv_obj_t *view = lv_cont_create(lv_scr_act(), nullptr);
    lv_obj_set_size(view, 240, 240);
    lv_obj_add_style(view, LV_OBJ_PART_MAIN, &cont_style);   
}

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {   
    //Only decode if package is relevant data
    if (length == 20) {
      if (pData[0] == 0xAA && pData[1] == 0x55) {
        decodeKS(pData); // For Kingsong only atm.
      }
    }
}

/*  Kingsong wheel data decoder adds current values to the wheeldata array. Prootocol decoding from Wheellog by 
 *  Cedric Hauber and Palachzzz, would have been a lot of work without that code to reference. Should be fairly
 *  easy to adapt this to Gotway as well. */ 
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
    if (KSdata[15] == 0xe0){
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
    // Battery percentage, using betterPercents algorithm from Wheellog
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
    
//Invoke the lvgl task handler from within the notifycallback handler 
//(updates the text labels everytime there is a valid notification from the characteristic FFE1)
  lv_task_handler(); 
} // End decodeKS

static int decode2byte(byte byte1, byte byte2) { //converts big endian 2 byte value to int
  int val;
  val = (byte1&0xFF) + (byte2<<8);
   return val;
}

static int decode4byte(byte byte1, byte byte2, byte byte3, byte byte4) { //converts bizarre 4 byte value to int
  float val;
  val = (byte1<<16) + (byte2<<24) + byte3 + (byte4<<8);
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
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
    }
    
// Register for notify   
    if(pRemoteCharacteristic->canNotify() == true){
      pRemoteCharacteristic->registerForNotify(notifyCallback);  
    }
    connected = true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
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

void initks() {
// Request Kingsong Model Name
  byte BLEreq[20] = {0x00};
  BLEreq[0] = 0xAA;  //Header byte 1
  BLEreq[1] = 0x55;  //Header byte 2
  BLEreq[16] = 0x9B; //This is the byte that specifies what data is requested
  BLEreq[17] = 0x14; //Last 3 bytes also needed
  BLEreq[18] = 0x5A;
  BLEreq[19] = 0x5A;
  pRemoteCharacteristic->writeValue(BLEreq, 20);
  delay(200);

 // Request Kingsong Serial Number
  BLEreq[16] = 0x63; //
  pRemoteCharacteristic->writeValue(BLEreq, 20);
  delay(200);

// Request Kingsong speed settings
  BLEreq[16] = 0x98;
  pRemoteCharacteristic->writeValue(BLEreq, 20);
} //End of initks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");



// Setup lvgl for t-watch 2020
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->lvgl_begin();
  setupGUI();
  Serial.println("starting lv task handler");
  lv_task_handler();
  lv_labels_create();

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

// main loop.
void loop() {
// If the flag "doConnect" is true then we have scanned for and found the desired
// BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
// connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
    
// Send necessary initialisation packages to start BLE notifications, do not know why this is needed though
   Serial.println("initialising KingSong");
   initks();
   }
  if (connected) {  
  }else if(doScan){
    BLEDevice::getScan()->start(0); 
  }
  delay(500); // Delay half a second between loops.
}
