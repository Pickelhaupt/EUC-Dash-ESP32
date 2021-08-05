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
#include "wheeldb.h"
#include "callback.h"
#include "json_psram_allocator.h"
#include "alloc.h"

byte KS_BLEreq[20];

void ks_ble_set(byte parameter, byte value);
void kingsong_decode_serial( void );

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

void setKSconstants()
{
    wheelctl_set_info(WHEELCTL_INFO_MODEL, "UNKNOWN");
    wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, KS_DEFAULT_MAXCURRENT);
    wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, KS_DEFAULT_CRITTEMP);
    wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, KS_DEFAULT_WARNTEMP);
    wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, KS_DEFAULT_BATTVOLT);
    wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, KS_DEFAULT_BATTWARN);
    wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, KS_DEFAULT_BATT_IR);
}


void decodeKS(byte KSdata[])
{
    //Parse incoming BLE Notifications
    if (KSdata[16] == 0xa9)
    { // Data package type 1 voltage/speed/odo/current/temperature
        int ks_current = decode2byte(KSdata[10], KSdata[11]);
        if (ks_current > 32767) ks_current = ks_current - 65536; // Ugly hack to display negative currents, must be a better way
        wheelctl_set_data(WHEELCTL_CURRENT, ks_current / 100.0); // Current must come before voltages for the battery calc to be correct
        wheelctl_set_data(WHEELCTL_VOLTAGE, (decode2byte(KSdata[2], KSdata[3]) / 100.0));
        wheelctl_set_data(WHEELCTL_SPEED, (decode2byte(KSdata[4], KSdata[5]) / 100.0));
        wheelctl_set_data(WHEELCTL_ODO, (decode4byte(KSdata[6], KSdata[7], KSdata[8], KSdata[9]) / 1000.0));
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
    else if (KSdata[16] == 0xb3) { //Serial number
        byte serial_no[18];
        for (int i = 2; i < 16; i++) {
            serial_no[i - 2] = KSdata[i];
        }
        serial_no[14] = KSdata[17];
        serial_no[15] = KSdata[18];
        serial_no[16] = KSdata[19];
        serial_no[17] = 0x00;
        char sstring[sizeof(serial_no) + 1];
        memcpy(sstring, serial_no, sizeof(serial_no));
        sstring[sizeof(serial_no)] = 0;
        wheelctl_set_info(WHEELCTL_INFO_SERIAL, sstring);
    }
    else if (KSdata[16] == 0xbb) { //name and model
        byte model_name[14];
        for (int i = 2; i < 16; i++) {
            if (KSdata[i] != 0) {
                model_name[i - 2] = KSdata[i];
            }
        }
        char mstring[sizeof(model_name) + 1];
        memcpy(mstring, model_name, sizeof(model_name));
        mstring[sizeof(model_name)] = 0;
        wheelctl_set_info(WHEELCTL_INFO_MODEL, mstring);
    }
} // End decodeKS

void kingsong_decode_serial( void ) {
    String ks_serialno = wheelctl_get_info(WHEELCTL_INFO_SERIAL);
    String ks_model = ks_serialno.substring(0,6);
    String ks_colour = ks_serialno.substring(6,7);
    String ks_manuf_date = ks_serialno.substring(7,13);

    Serial.print("Model: ");
    Serial.println(ks_model);

    if (ks_model == "KS14D0") wheeldb_set_wheelctl_data(KS14D0);
    else if (ks_model == "KS14D1") wheeldb_set_wheelctl_data(KS14D1);
    else if (ks_model == "KS14D2") wheeldb_set_wheelctl_data(KS14D2);
    else if (ks_model == "KS14D3") wheeldb_set_wheelctl_data(KS14D3);
    else if (ks_model == "KS14D4") wheeldb_set_wheelctl_data(KS14D4);
    else if (ks_model == "KS16S1") wheeldb_set_wheelctl_data(KS16S1);
    else if (ks_model == "KS16S2") wheeldb_set_wheelctl_data(KS16S2);
    else if (ks_model == "KS16S4") wheeldb_set_wheelctl_data(KS16S4);
    else if (ks_model == "KS16X1") wheeldb_set_wheelctl_data(KS16X1);
    else if (ks_model == "KS16X2") wheeldb_set_wheelctl_data(KS16X2);
    else if (ks_model == "KSS181") wheeldb_set_wheelctl_data(KSS181);
    else if (ks_model == "KS18L2") wheeldb_set_wheelctl_data(KS18L2);
    else if (ks_model == "KS18L4") wheeldb_set_wheelctl_data(KS18L4);
    else {
        wheeldb_set_wheelctl_data(KSUNKN);
        Serial.println("unknown KS model -- please add");
    }

    if (ks_colour == "B") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "black");
    if (ks_colour == "C") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "custom");
    if (ks_colour == "D") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "black rubber");
    if (ks_colour == "R") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "red");
    if (ks_colour == "S") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "silver");
    if (ks_colour == "W") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "white");
    if (ks_colour == "Y") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "yellow");

}

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

void ks_lights(byte mode) { //0=on, 1=off
    byte byte2 = 0x12 + mode;
    ks_ble_set(0x73, byte2);
}

/*
void ks_lock(void) { 
    ks_ble_set(0x5d, 0x01);
}

void ks_unlock(byte passcode[6]) { 
    ks_ble_set(0x5d, 0x00);
}
*/

void ks_led(byte mode) { //check actual values
    byte byte2 = mode;
    ks_ble_set(0x6C, byte2);
}

void ks_pedals(byte mode) { //0=hard, 1=med, 2=soft
    byte byte2 = mode; //check this before use
    ks_ble_set(0x87, byte2);
}

void ks_ble_set(byte parameter, byte value)
{
    /****************************************************************
      parameter is the byte representing the parameter to be set
      0x73 -- Lights (0x12, 0x13, 0x14, 0x15) value 2 = 0x01
      0x87 -- pedals mode (0x00, 0x01, 0x02) value = 0xE0
      0x53 -- strobemode (0x00 or 0x01) value2 = 0x00
      0x6C -- side led mode value2 = 0x00
      0x5d -- lock wheel. value 0x00 unlock, 0x01 lock
   *****************************************************************/
    byte value2{0x00};
    if (parameter == 0x73) value2 = 0x01;
    if (parameter == 0x87) value2 = 0xE0;

    byte KS_BLEreq[20] = {0x00}; //set array to zero
    KS_BLEreq[0] = 0xAA;         //Header byte 1
    KS_BLEreq[1] = 0x55;         //Header byte 2
    KS_BLEreq[2] = value;
    KS_BLEreq[3] = value2;      //only required for certain settingd
    KS_BLEreq[16] = parameter;  // This is the byte that specifies what data is requested
    KS_BLEreq[17] = 0x14;       //Last 3 bytes also needed
    KS_BLEreq[18] = 0x5A;
    KS_BLEreq[19] = 0x5A;
    writeBLE(KS_BLEreq, 20);
}

void ks_init_notifications() {
    log_i("requesting model..");
    ks_ble_request(0x9B);
    delay(200);
    log_i("requesting serial..");
    ks_ble_request(0x63);
    delay(200);
    log_i("requesting speed settings..");
    ks_ble_request(0x98);
    delay(200);
}

void initks()
{
    //Setting of some model specific parametes,
    setKSconstants();
    ks_init_notifications();
    kingsong_decode_serial();
    wheelctl_connect_actions();
} //End of initks