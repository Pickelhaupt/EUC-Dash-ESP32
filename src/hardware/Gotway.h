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

#ifndef __GOTWAY_H
#define __GOTWAY_H

/*define constants for specific GW wheel models 
* add here when adding support for a new model
*/
#define GW_DEFAULT_MAXCURRENT   45 //maximum current draw
#define GW_DEFAULT_CRITTEMP     65 //critical internal temperature
#define GW_DEFAULT_WARNTEMP     50 //Hight internal temperature
#define GW_DEFAULT_BATTVOLT     84 //battery pack max voltage
#define GW_DEFAULT_BATTWARN     35 //remaining battery percentage warning -- 
#define GW_DEFAULT_BATT_IR     15 //internal resistance in 1/100 ohms -- 

#define GW_MCM5_MAXCURRENT   45 //maximum current draw
#define GW_MCM5_CRITTEMP     65 //critical internal temperature
#define GW_MCM5_WARNTEMP     50 //High internal temperature
#define GW_MCM5_BATTVOLT     84 //battery pack max voltage
#define GW_MCM5_BATTWARN     30 //remaining battery percentage warning
#define GW_MCM5_BATT_IR      15 //battery pack internal resistance in 1/100 ohms -- just a guess at the moment, will need tuning

#define GW_TESLA_MAXCURRENT   50
#define GW_TESLA_CRITTEMP     65
#define GW_TESLA_WARNTEMP     50
#define GW_TESLA_BATTVOLT     84
#define GW_TESLA_BATTWARN     25
#define GW_TESLA_BATT_IR      15

#define GW_TESLAV2_MAXCURRENT   50
#define GW_TESLAV2_CRITTEMP     65
#define GW_TESLAV2_WARNTEMP     50
#define GW_TESLAV2_BATTVOLT     84
#define GW_TESLAV2_BATTWARN     25
#define GW_TESLAV2_BATT_IR      15

#define GW_NIKOLA_MAXCURRENT   50
#define GW_NIKOLA_CRITTEMP     65
#define GW_NIKOLA_WARNTEMP     50
#define GW_NIKOLA_BATTVOLT     84
#define GW_NIKOLA_BATTWARN     25
#define GW_NIKOLA_BATT_IR      15


#include <TTGO.h>
#include "callback.h"
/**
* @brief Set the primary lights mode 
* 
* @param   mode     configitem: byte containing the desired light mode: 0x00 = on, 0x01 = off, 0x02 = ? 0x03 = ?
* 
*/
void gw_lights(byte mode);

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

/**
* @brief Decode a kingsong 20 byte BLE package assigned to a byte array 
* 
* @param   GWData[]     configitem: byte array containing 20 elements
* 
*/
void decodeGW(byte GWData[]);
/**
* @brief Initialise the Kingsong BLE notification function by sending a series of requests
*/
void initgw();

#endif /* __KINGSONG */