/****************************************************************************
 * 2020 Jesper Ortlund
 * KingSong decoding derived from Wheellog 
 ****************************************************************************/
 
/****************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *****************************************************************************/

#include <stdio.h>
#include <time.h>

#include "config.h"
#include "Arduino.h"
#include "Kingsong.h"

#include "blectl.h"
#include "wheelctl.h"
#include "callback.h"
#include "json_psram_allocator.h"
#include "alloc.h"


int add_ride_millis (void);

byte KS_BLEreq[20];

float wheeldata[17];
//float wheeldata[16] = {67.1, 12.3, 23.4, 8.4, 33.2, 0.0, 88.3, 543.0, 46.7, 443, 28.4, 0.0, 0.0, 0.0, 28.0, 30.0};

float max_speed = 0;
float avg_speed = 0;
float max_batt = 0;
float min_batt = 100;
float max_current = 0;
float regen_current = 0;
float max_temp = 0;
String wheelmodel = "KS14D";
struct Wheel_constants wheelconst;
//remove when BLE_CLI is done
bool tempconn = false;

/**************************************************
   Decode big endian multi byte data from KS wheels
 **************************************************/
static int decode2byte(byte byte1, byte byte2)
{ //converts big endian 2 byte value to int
    int val;
    val = (byte1 & 0xFF) + (byte2 << 8);
    return val;
}

static int decode4byte(byte byte1, byte byte2, byte byte3, byte byte4)
{ //converts bizarre 4 byte value to int
    int val;
    val = (byte1 << 16) + (byte2 << 24) + byte3 + (byte4 << 8);
    return val;
}


void setKSconstants(String model)
{
    if (model = "KS14D")
    {
        wheelconst.maxcurrent = KS_14D_MAXCURRENT;
        wheelconst.crittemp = KS_14D_CRITTEMP;
        wheelconst.warntemp = KS_14D_WARNTEMP;
        wheelconst.battvolt = KS_14D_BATTVOLT;
        wheelconst.battwarn = KS_14D_BATTWARN;
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_14D_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_14D_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_14D_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_14D_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_14D_BATTWARN);
    }
    else if (model = "KS14S")
    {
        wheelconst.maxcurrent = KS_14S_MAXCURRENT;
        wheelconst.crittemp = KS_14S_CRITTEMP;
        wheelconst.warntemp = KS_14S_WARNTEMP;
        wheelconst.battvolt = KS_14S_BATTVOLT;
        wheelconst.battwarn = KS_14S_BATTWARN;
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_14S_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_14S_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_14S_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_14S_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_14S_BATTWARN);
    }
    else if (model = "KS16S")
    {
        wheelconst.maxcurrent = KS_16S_MAXCURRENT;
        wheelconst.crittemp = KS_16S_CRITTEMP;
        wheelconst.warntemp = KS_16S_WARNTEMP;
        wheelconst.battvolt = KS_16S_BATTVOLT;
        wheelconst.battwarn = KS_16S_BATTWARN;
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_16S_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_16S_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_16S_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_16S_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_16S_BATTWARN);
    }
    else if (model = "KS16X")
    {
        wheelconst.maxcurrent = KS_16X_MAXCURRENT;
        wheelconst.crittemp = KS_16X_CRITTEMP;
        wheelconst.warntemp = KS_16X_WARNTEMP;
        wheelconst.battvolt = KS_16X_BATTVOLT;
        wheelconst.battwarn = KS_16X_BATTWARN;
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_16X_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_16X_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_16X_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_16X_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_16X_BATTWARN);
    }
    else
    {
        wheelconst.maxcurrent = KS_DEFAULT_MAXCURRENT;
        wheelconst.crittemp = KS_DEFAULT_CRITTEMP;
        wheelconst.warntemp = KS_DEFAULT_WARNTEMP;
        wheelconst.battvolt = KS_DEFAULT_BATTVOLT;
        wheelconst.battwarn = KS_DEFAULT_BATTWARN;
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_DEFAULT_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_DEFAULT_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_DEFAULT_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_DEFAULT_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_DEFAULT_BATTWARN);
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
void decodeKS(byte KSdata[])
{

    int rMode = 0;
    float Battpct;
    float negamp;

    //Parse incoming BLE Notifications
    if (KSdata[16] == 0xa9)
    { // Data package type 1 voltage/speed/odo/current/temperature
        wheelctl_set_data(WHEELCTL_VOLTAGE, (decode2byte(KSdata[2], KSdata[3]) / 100.0));
        wheelctl_set_data(WHEELCTL_SPEED, (decode2byte(KSdata[4], KSdata[5]) / 100.0));
        wheelctl_set_data(WHEELCTL_ODO, (decode4byte(KSdata[6], KSdata[7], KSdata[8], KSdata[9]) / 1000.0));
        wheelctl_set_data(WHEELCTL_CURRENT,  (decode2byte(KSdata[10], KSdata[11]) / 100.0));
        wheelctl_set_data(WHEELCTL_TEMP, (decode2byte(KSdata[12], KSdata[13]) / 100.0));
        wheelctl_set_data(WHEELCTL_RMODE, KSdata[14]); // check this!!

        int rBattvolt = decode2byte(KSdata[2], KSdata[3]);
        int rSpeed = decode2byte(KSdata[4], KSdata[5]);
        int rOdo = decode4byte(KSdata[6], KSdata[7], KSdata[8], KSdata[9]);
        int rCurr = decode2byte(KSdata[10], KSdata[11]);
        int rTemp = decode2byte(KSdata[12], KSdata[13]);
        if (KSdata[15] == 0xe0)
        {
            rMode = KSdata[14];
        }
        // Convert to readable numbers and add to the wheeldata array
        wheeldata[0] = rBattvolt / 100.00; //Battry voltage
        wheeldata[1] = rSpeed / 100.00;    //Current speed
        wheeldata[2] = rOdo / 1000.00;     //Total kms ridden
        if (rCurr > 32767)
        { // Ugly hack to display negative currents, must be a better way
            rCurr = rCurr - 65536;
        }
        wheeldata[3] = rCurr / 100.00; //Current Current :)
        wheeldata[4] = rTemp / 100.00; //Current wheel temperature
        wheeldata[5] = rMode;
        // Battery percentage, using betterPercents algorithm from Wheellog, only for 67V atm
        if (rBattvolt > 6680)
        {
            Battpct = 100;
        }
        else if (rBattvolt > 5440)
        {
            Battpct = (rBattvolt - 5320) / 13.6;
        }
        else if (rBattvolt > 5120)
        {
            Battpct = (rBattvolt - 5120) / 36.0;
        }
        else
        {
            Battpct = 0;
        }
        wheeldata[6] = Battpct;
        wheeldata[7] = wheeldata[0] * wheeldata[3]; //current power usage
    }
    else if (KSdata[16] == 0xb9)
    {   // Data package type 2 distance/time/top speed/fan
        int rDistance = decode4byte(KSdata[2], KSdata[3], KSdata[4], KSdata[5]); // trip counter resets when wheel off
        int rWheeltime = decode2byte(KSdata[6], KSdata[7]);                      //time since turned on
        int rTopspeed = decode2byte(KSdata[8], KSdata[9]);                       //Max speed since last power on
        int rFanstate = KSdata[12];                                              //1 if fan is running

        wheeldata[8] = rDistance / 1000.0; //convert from m to km
        wheeldata[9] = rWheeltime;
        wheeldata[10] = rTopspeed / 100.0;
        wheeldata[11] = rFanstate;

        wheelctl_set_data(WHEELCTL_TRIP, (decode4byte(KSdata[2], KSdata[3], KSdata[4], KSdata[5]) / 1000.0)); // trip counter resets when wheel off
        wheelctl_set_data(WHEELCTL_UPTIME, (decode2byte(KSdata[6], KSdata[7]) / 100.0)); //time since turned on
        wheelctl_set_data(WHEELCTL_TOPSPEED, (decode2byte(KSdata[8], KSdata[9]) / 100.0)); //Max speed since last power on
        wheelctl_set_data(WHEELCTL_FANSTATE,  KSdata[12]); //1 if fan is running

    }
    else if (KSdata[16] == 0xb5)
    {   //Data package type 3 speed alarms and tiltback speed
        wheeldata[12] = KSdata[4];  //Alarmspeed 1
        wheeldata[13] = KSdata[6];  //Alarmspeed 2
        wheeldata[14] = KSdata[8];  //Alarmspeed 3
        wheeldata[15] = KSdata[10]; //Max speed (tiltback)

        wheelctl_set_data(WHEELCTL_ALARM1,  KSdata[4]);
        wheelctl_set_data(WHEELCTL_ALARM2,  KSdata[6]);
        wheelctl_set_data(WHEELCTL_ALARM3,  KSdata[8]);
        wheelctl_set_data(WHEELCTL_TILTBACK,  KSdata[10]);
    }

    //set values for max/min arc bars
    if (wheeldata[10] < (wheeldata[15] + 5))
    {
        max_speed = wheeldata[10];
    }
    else
    {
        max_speed = (wheeldata[15] + 5);
    }
    if (wheeldata[6] > max_batt && wheeldata[3] >= 0)
    {
        max_batt = wheeldata[6];
    }
    if (wheeldata[6] < min_batt && wheeldata[6] != 0)
    {
        min_batt = wheeldata[6];
    }
    if (wheeldata[3] > max_current && wheeldata[3] <= wheelconst.maxcurrent)
    {
        max_current = wheeldata[3];
    }
    if (wheeldata[3] < 0)
    {
        negamp = (wheeldata[3] * -1);
        if (negamp > regen_current)
        {
            regen_current = negamp;
        }
    }
    if (wheeldata[4] > max_temp)
    {
        max_temp = wheeldata[4];
    }

    wheeldata[16] = add_ride_millis();
    wheelctl_set_data(WHEELCTL_RIDETIME,  (add_ride_millis() / 1000));
    //Debug -- testing, print all data to serial
    //Serial.print(wheeldata[0]); Serial.println(" V");
    //Serial.print(wheeldata[1]); Serial.println(" kmh");
    // Serial.print(wheeldata[2]); Serial.println(" km");
    //Serial.print(wheeldata[3]); Serial.println(" A");
    //Serial.print(wheeldata[4]); Serial.println(" C");
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
    //Serial.print(wheeldata[15]); Serial.println(" maxspeed");
    //Serial.print(wheeldata[16]); Serial.println(" riding seconds");
    //Serial.print(max_speed); Serial.println(" max_speed");
    //Serial.print(max_batt); Serial.println(" max_batt");
    //Serial.print(min_batt); Serial.println(" min_batt");
    //Serial.print(max_current); Serial.println(" max_current");
    //Serial.print(max_temp); Serial.println(" max_temp");
} // End decodeKS

void ks_ble_request(byte reqtype)
{
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
    KS_BLEreq[0] = 0xAA;         //Header byte 1
    KS_BLEreq[1] = 0x55;         //Header byte 2
    KS_BLEreq[16] = reqtype;     // This is the byte that specifies what data is requested
    KS_BLEreq[17] = 0x14;        //Last 3 bytes also needed
    KS_BLEreq[18] = 0x5A;
    KS_BLEreq[19] = 0x5A;
    writeBLE(KS_BLEreq, 20);
}

int add_ride_millis () {
    static unsigned long ride_millis;
    static unsigned long old_millis = 0;
    if (wheeldata[1] > 1) {
        ride_millis =+ (millis() - old_millis);
    }
    old_millis = millis();
    return ride_millis;
}

void initks()
{
    //temporary model strings, Todo: implement automated id
    //wheelmodel = "KS16S";
    //Setting of some model specific parametes,
    //Todo: automatic model identification
    setKSconstants(wheelmodel);
    /*****************************************
       Request Kingsong Model Name, serial number and speed settings
       This must be done before any BLE notifications will be pused by the KS wheel
  ******************************************/
    //TTGOClass *ttgo = TTGOClass::getWatch();
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