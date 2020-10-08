#include "twatch_config.h"
#include "EUCDash.h"
#include "BLEDevice.h"

byte KS_BLEreq[20];
extern float wheeldata[];
float max_speed = 0;
float avg_speed = 0;
float max_batt = 0;
float min_batt = 100;
float max_current = 0;
float max_temp = 0;



/**************************************************
   Decode big endian multi byte data from KS wheels
 **************************************************/
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

void setKSconstants(void) {
  //struct Wheel_constants wheelconst;
  if (wheelmodel = "KS14SMD") {
    wheelconst.maxcurrent = 35;
    wheelconst.crittemp = 65;
    wheelconst.warntemp = 50;
    wheelconst.battvolt = 67;
    wheelconst.battwarn = 40;
  } else if (wheelmodel = "KS16S") {
    wheelconst.maxcurrent = 40;
    wheelconst.crittemp = 65;
    wheelconst.warntemp = 50;
    wheelconst.battvolt = 67;
    wheelconst.battwarn = 30;
  } else if (wheelmodel = "KS16X") {
    wheelconst.maxcurrent = 45;
    wheelconst.crittemp = 65;
    wheelconst.warntemp = 50;
    wheelconst.battvolt = 82;
    wheelconst.battwarn = 20;
  } else {
    wheelconst.maxcurrent = 40;
    wheelconst.crittemp = 65;
    wheelconst.warntemp = 50;
    wheelconst.battvolt = 67;
    wheelconst.battwarn = 30;
  }
}

/*************************************************************
    Kingsong wheel data decoder adds current values to
    the wheeldata array. Prootocol decoding from Wheellog by
    Kevin Cooper,
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
void decodeKS (byte KSdata[]) {

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

  //set values for max/min arc bars
  if (wheeldata[10] < (wheeldata[15] + 5)) {
    max_speed = wheeldata[10];
  } else {
    max_speed = (wheeldata[15] + 5);
  }
  if (wheeldata[6] > max_batt && wheeldata[3] >= 0) {
    max_batt = wheeldata[6];
  }
  if (wheeldata[6] < min_batt && wheeldata[6] != 0) {
    min_batt = wheeldata[6];
  }
  if (wheeldata[3] > max_current && wheeldata[3] <= wheelconst.maxcurrent) {
    max_current = wheeldata[3];
  }
  if (wheeldata[4] > max_temp) {
    max_temp = wheeldata[4];
  }
  //Debug -- testing, print all data to serial
  //Serial.print(wheeldata[0]); Serial.println(" V");
  //Serial.print(wheeldata[1]); Serial.println(" kmh");
  // Serial.print(wheeldata[2]); Serial.println(" km");
  //Serial.print(wheeldata[3]); Serial.println(" A");
  // Serial.print(wheeldata[4]); Serial.println(" C");
  //Serial.print(wheeldata[5]); Serial.println(" rmode");
  // Serial.print(wheeldata[6]); Serial.println(" %");
  // Serial.print(wheeldata[7]); Serial.println(" W");
  // Serial.print(wheeldata[8]); Serial.println(" km");
  // Serial.print(wheeldata[9]); Serial.println(" time");
  //Serial.print(wheeldata[10]); Serial.println(" kmh");
  // Serial.print(wheeldata[11]); Serial.println(" fan");
  //Serial.print(wheeldata[12]); Serial.println(" alarm1");
  //Serial.print(wheeldata[13]); Serial.println(" alarm2");
  //Serial.print(wheeldata[14]); Serial.println(" alarm3");
  // Serial.print(wheeldata[15]); Serial.println(" maxspeed");
  //Serial.print(max_speed); Serial.println(" max_speed");
  //Serial.print(max_batt); Serial.println(" max_batt");
  //Serial.print(min_batt); Serial.println(" min_batt");
  //Serial.print(max_current); Serial.println(" max_current");
  //Serial.print(max_temp); Serial.println(" max_temp");
  if (connected && displayOff) {
    lv_task_handler();
  }
} // End decodeKS


void ks_ble_request(byte reqtype) {
  /****************************************************************
      reqtype is the byte representing the request id
      0x9B -- Serial Number
      0x63 -- Manufacturer and model
      0x98 -- speed alarm settings and tiltback (Max) speed
      0x88 -- horn
      Responses to the request is handled by the notification handler
      and will be added to the wheeldata[] array
   *****************************************************************/
  byte KS_BLEreq[20] = {0x00}; //set array to zero
  KS_BLEreq[0] = 0xAA;  //Header byte 1
  KS_BLEreq[1] = 0x55;  //Header byte 2
  KS_BLEreq[16] = reqtype;  // This is the byte that specifies what data is requested
  KS_BLEreq[17] = 0x14; //Last 3 bytes also needed
  KS_BLEreq[18] = 0x5A;
  KS_BLEreq[19] = 0x5A;
  writeBLE(KS_BLEreq, 20);
}

void initks() {
  if (Wheel_brand = "KingSong") {
    //temporary model strings, Todo: implement automated id
    wheelmodel = "KS14SMD";
    //wheelmodel = "KS16S";
  }
  //Setting of some model specific parametes,
  //Todo: automatic model identification
  setKSconstants();
  /*****************************************
       Request Kingsong Model Name, serial number and speed settings
       This must be done before any BLE notifications will be pused by the KS wheel
  ******************************************/
  //TTGOClass *ttgo = TTGOClass::getWatch();
  Serial.println("requesting model..");
  ks_ble_request(0x9B);
  Serial.println("requesting serial..");
  ks_ble_request(0x63);
  Serial.println("requesting speed settings..");
  ks_ble_request(0x98);
} //End of initks
