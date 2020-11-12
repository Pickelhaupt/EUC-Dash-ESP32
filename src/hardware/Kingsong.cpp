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
String wheelmodel = "KS14D";

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
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_14D_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_14D_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_14D_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_14D_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_14D_BATTWARN);
    }
    else if (model = "KS14S")
    {
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_14S_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_14S_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_14S_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_14S_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_14S_BATTWARN);
    }
    else if (model = "KS16S")
    {
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_16S_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_16S_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_16S_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_16S_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_16S_BATTWARN);
    }
    else if (model = "KS16X")
    {
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_16X_MAXCURRENT);
        wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_16X_CRITTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_16X_WARNTEMP);
        wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_16X_BATTVOLT);
        wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_16X_BATTWARN);
    }
    else
    {
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
    Function is called on by the notifyCallback function in blectl
    decoded data is added to the data struct in wheelctl
    Todo:
    - Add Serial and model number
    - Add periodical polling of speed settings? Verify by
      testing with low battery
 ************************************************************/
void decodeKS(byte KSdata[])
{
    //Parse incoming BLE Notifications
    if (KSdata[16] == 0xa9)
    { // Data package type 1 voltage/speed/odo/current/temperature
        float rCurr = decode2byte(KSdata[10], KSdata[11]) / 100.0;
        if (rCurr > 32767) rCurr = rCurr - 65536; // Ugly hack to display negative currents, must be a better way
      
        wheelctl_set_data(WHEELCTL_VOLTAGE, (decode2byte(KSdata[2], KSdata[3]) / 100.0));
        wheelctl_set_data(WHEELCTL_SPEED, (decode2byte(KSdata[4], KSdata[5]) / 100.0));
        wheelctl_set_data(WHEELCTL_ODO, (decode4byte(KSdata[6], KSdata[7], KSdata[8], KSdata[9]) / 1000.0));
        wheelctl_set_data(WHEELCTL_CURRENT, rCurr);
        wheelctl_set_data(WHEELCTL_TEMP, (decode2byte(KSdata[12], KSdata[13]) / 100.0));
        wheelctl_set_data(WHEELCTL_RMODE, KSdata[14]); // check this!!
    }
    else if (KSdata[16] == 0xb9)
    {   // Data package type 2 distance/time/top speed/fan
        wheelctl_set_data(WHEELCTL_TRIP, (decode4byte(KSdata[2], KSdata[3], KSdata[4], KSdata[5]) / 1000.0)); // trip counter resets when wheel off
        wheelctl_set_data(WHEELCTL_UPTIME, (decode2byte(KSdata[6], KSdata[7]) / 100.0)); //time since turned on
        wheelctl_set_data(WHEELCTL_TOPSPEED, (decode2byte(KSdata[8], KSdata[9]) / 100.0)); //Max speed since last power on
        wheelctl_set_data(WHEELCTL_FANSTATE, KSdata[12]); //1 if fan is running
    }
    else if (KSdata[16] == 0xb5)
    {   //Data package type 3 speed alarms and tiltback speed
        wheelctl_set_data(WHEELCTL_ALARM1, KSdata[4]);
        wheelctl_set_data(WHEELCTL_ALARM2, KSdata[6]);
        wheelctl_set_data(WHEELCTL_ALARM3, KSdata[8]);
        wheelctl_set_data(WHEELCTL_TILTBACK, KSdata[10]);
    }
    wheelctl_set_data(WHEELCTL_RIDETIME,  (add_ride_millis() / 1000));
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
      and will be added to the wheel data handled by wheelctl
      todo -- find out how to toggle lights and add function
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
    if (wheelctl_get_data(WHEELCTL_SPEED) > 1) {
        ride_millis =+ (millis() - old_millis);
    }
    old_millis = millis();
    return ride_millis;
}

void initks()
{
    //temporary model strings, Todo: implement automated id;
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