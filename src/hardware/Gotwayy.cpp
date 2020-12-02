/****************************************************************************
 * 2020 Jesper Ortlund
 * Gotway decoding derived from Wheellog 
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
#include "Gotway.h"

#include "blectl.h"
#include "wheelctl.h"
#include "callback.h"
#include "json_psram_allocator.h"
#include "alloc.h"

byte GW_BLEreq[20];
String gw_wheelmodel = "GW14D";

void gw_ble_set(byte parameter, byte value);
void gotway_decode_serial( void );

/**************************************************
   Decode big endian multi byte data from GW wheels
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

void setGWconstants()
{
    wheelctl_set_info(WHEELCTL_INFO_MODEL, "UNKNOWN");
    wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, GW_DEFAULT_MAXCURRENT);
    wheelctl_set_constant(WHEELCTL_CONST_CRITTEMP, GW_DEFAULT_CRITTEMP);
    wheelctl_set_constant(WHEELCTL_CONST_WARNTEMP, GW_DEFAULT_WARNTEMP);
    wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, GW_DEFAULT_BATTVOLT);
    wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, GW_DEFAULT_BATTWARN);
    wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, GW_DEFAULT_BATT_IR);
}


void decodeGW(byte GWdata[])
{
    //Parse incoming BLE Notifications
    if (GWdata[16] == 0xa9)
    { // Data package type 1 voltage/speed/odo/current/temperature
        int gw_current = decode2byte(GWdata[10], GWdata[11]);
        if (gw_current > 32767) gw_current = gw_current - 65536; // Ugly hack to display negative currents, must be a better way
        wheelctl_set_data(WHEELCTL_CURRENT, gw_current / 100.0); // Current must come before voltages for the battery calc to be correct
        wheelctl_set_data(WHEELCTL_VOLTAGE, (decode2byte(GWdata[2], GWdata[3]) / 100.0));
        wheelctl_set_data(WHEELCTL_SPEED, (decode2byte(GWdata[4], GWdata[5]) / 100.0));
        wheelctl_set_data(WHEELCTL_ODO, (decode4byte(GWdata[6], GWdata[7], GWdata[8], GWdata[9]) / 1000.0));
        wheelctl_set_data(WHEELCTL_TEMP, (decode2byte(GWdata[12], GWdata[13]) / 100.0));
        wheelctl_set_data(WHEELCTL_RMODE, GWdata[14]); // check this!!
    }
    else if (GWdata[16] == 0xb9)
    {   // Data package type 2 distance/time/top speed/fan
        wheelctl_set_data(WHEELCTL_TRIP, (decode4byte(GWdata[2], GWdata[3], GWdata[4], GWdata[5]) / 1000.0)); // trip counter resets when wheel off
        wheelctl_set_data(WHEELCTL_UPTIME, (decode2byte(GWdata[6], GWdata[7]) / 100.0)); //time since turned on
        wheelctl_set_data(WHEELCTL_TOPSPEED, (decode2byte(GWdata[8], GWdata[9]) / 100.0)); //Max speed since last power on
        wheelctl_set_data(WHEELCTL_FANSTATE, GWdata[12]); //1 if fan is running
    }
    else if (GWdata[16] == 0xb5)
    {   //Data package type 3 speed alarms and tiltback speed
        wheelctl_set_data(WHEELCTL_ALARM1, GWdata[4]);
        wheelctl_set_data(WHEELCTL_ALARM2, GWdata[6]);
        wheelctl_set_data(WHEELCTL_ALARM3, GWdata[8]);
        wheelctl_set_data(WHEELCTL_TILTBACK, GWdata[10]);
    }
    else if (GWdata[16] == 0xb3) { //Serial number
        byte serial_no[18];
        for (int i = 2; i < 16; i++) {
            serial_no[i - 2] = GWdata[i];
        }
        serial_no[14] = GWdata[17];
        serial_no[15] = GWdata[18];
        serial_no[16] = GWdata[19];
        serial_no[17] = 0x00;
        char sstring[sizeof(serial_no) + 1];
        memcpy(sstring, serial_no, sizeof(serial_no));
        sstring[sizeof(serial_no)] = 0;
        wheelctl_set_info(WHEELCTL_INFO_SERIAL, sstring);
    }
    else if (GWdata[16] == 0xbb) { //name and model
        byte model_name[14];
        for (int i = 2; i < 16; i++) {
            if (GWdata[i] != 0) {
                model_name[i - 2] = GWdata[i];
            }
        }
        char mstring[sizeof(model_name) + 1];
        memcpy(mstring, model_name, sizeof(model_name));
        mstring[sizeof(model_name)] = 0;
        wheelctl_set_info(WHEELCTL_INFO_MODEL, mstring);
    }
} // End decodeGW

void gotway_decode_serial( void ) {
    String gw_serialno = wheelctl_get_info(WHEELCTL_INFO_SERIAL);
    String gw_size = gw_serialno.substring(0,4);
    String gw_battsize = gw_serialno.substring(4,6);
    String gw_colour = gw_serialno.substring(6,7);
    String gw_manuf_date = gw_serialno.substring(7,13);

    if (gw_size == "GW14") {
        wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 35);
        if (gw_battsize == "D0") { 
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "174");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 1);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW14M");
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 25);
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 50);
        }
        else if (gw_battsize == "D1") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "340");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 2);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW14D");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 40);
        }
        else if (gw_battsize == "D2") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "420");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 2);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW14D");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 40);
        }
        else if (gw_battsize == "D3") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW14S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
        }
        else if (gw_battsize == "D4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "840");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW14S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
        }
    }
    else if (gw_size == "GW16"){
        if (gw_battsize == "D1") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "320");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 2);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 30);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 35);
        }
        else if (gw_battsize == "D2") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "420");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 2);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 30);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 35);
        }
        else if (gw_battsize == "D3") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "D4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "840");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "D5") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "520");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 3);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 30);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 40);
        }
        else if (gw_battsize == "S1") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "S2") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "840");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "S4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "420");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 2);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 40);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 35);
        }
        else if (gw_battsize == "X1") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "777");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 84);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 3);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16XS");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "X2") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1554");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 84);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 6);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW16X");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 50);
        }
    }
    else if (gw_size == "GW18"){
        if (gw_battsize == "A4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "520");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 3);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18A");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 30);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "A5") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18A");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "A6") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1360");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 8);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18A");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 55);
        }
        else if (gw_battsize == "A7") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "840");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18A");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "A8") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 8);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18A");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 50);
        }

        else if (gw_battsize == "S3") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 50);
        }
        else if (gw_battsize == "S4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1360");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 6);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 55);
        }
        else if (gw_battsize == "S5") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "840");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 50);
        }
        else if (gw_battsize == "S6") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1680");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 67);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 8);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18S");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 55);
        }
        else if (gw_battsize == "L2") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1036");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 84);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 4);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18L");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 45);
        }
        else if (gw_battsize == "L4") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1554");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 84);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 6);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW18XL");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 20);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 55);
        }
    }
    else if (gw_size == "GWS1"){
        if (gw_battsize == "81") {
            wheelctl_set_info(WHEELCTL_INFO_BATTCAP, "1110");
            wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, 84);
            wheelctl_set_constant(WHEELCTL_CONST_BATT_P, 3);
            wheelctl_set_info(WHEELCTL_INFO_MODEL, "GW-S18");
            wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, 25);
            wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, 50);
        }
    }
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 1) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 30);
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 2) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 23);
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 3) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 20);
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 4) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 15);
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 6) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 13);
    if (wheelctl_get_constant(WHEELCTL_CONST_BATT_P) == 8) wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, 12);

    if (gw_colour == "B") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "black");
    if (gw_colour == "C") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "custom");
    if (gw_colour == "D") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "black rubber");
    if (gw_colour == "R") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "red");
    if (gw_colour == "S") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "silver");
    if (gw_colour == "W") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "white");
    if (gw_colour == "Y") wheelctl_set_info(WHEELCTL_INFO_WHEELCOLOR, "yellow");
}

void gw_ble_request(byte reqtype)
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
    byte GW_BLEreq[20] = {0x00}; //set array to zero
    GW_BLEreq[0] = 0xAA;         //Header byte 1
    GW_BLEreq[1] = 0x55;         //Header byte 2
    GW_BLEreq[16] = reqtype;     // This is the byte that specifies what data is requested
    GW_BLEreq[17] = 0x14;        //Last 3 bytes also needed
    GW_BLEreq[18] = 0x5A;
    GW_BLEreq[19] = 0x5A;
    writeBLE(GW_BLEreq, 20);
}

void gw_lights(byte mode) { //0=on, 1=off
    byte byte2 = 0x12 + mode;
    gw_ble_set(0x73, byte2);
}

void gw_led(byte mode) { //check actual values
    byte byte2 = mode;
    gw_ble_set(0x6C, byte2);
}

void gw_pedals(byte mode) { //0=hard, 1=med, 2=soft
    byte byte2 = mode; //check this before use
    gw_ble_set(0x87, byte2);
}

void gw_ble_set(byte parameter, byte value)
{
    /****************************************************************
      parameter is the byte representing the parameter to be set
      0x73 -- Lights (0x12, 0x13, 0x14, 0x15) value 2 = 0x01
      0x87 -- pedals mode (0x00, 0x01, 0x02) value = 0xE0
      0x53 -- strobemode (0x00 or 0x01) value2 = 0x00
      0x6C -- side led mode value2 = 0x00
   *****************************************************************/
    byte value2{0x00};
    if (parameter == 0x73) value2 = 0x01;
    if (parameter == 0x87) value2 = 0xE0;

    byte GW_BLEreq[20] = {0x00}; //set array to zero
    GW_BLEreq[0] = 0xAA;         //Header byte 1
    GW_BLEreq[1] = 0x55;         //Header byte 2
    GW_BLEreq[2] = value;
    GW_BLEreq[3] = value2;      //only required for certain settingd
    GW_BLEreq[16] = parameter;  // This is the byte that specifies what data is requested
    GW_BLEreq[17] = 0x14;       //Last 3 bytes also needed
    GW_BLEreq[18] = 0x5A;
    GW_BLEreq[19] = 0x5A;
    writeBLE(GW_BLEreq, 20);
}

void initgw()
{
    //Setting of some model specific parametes,
    setGWconstants();

    Serial.println("requesting model..");
    gw_ble_request(0x9B);
    delay(200);
    Serial.println("requesting serial..");
    gw_ble_request(0x63);
    delay(200);
    Serial.println("requesting speed settings..");
    gw_ble_request(0x98);
    delay(200);

    gotway_decode_serial();
} //End of initgw