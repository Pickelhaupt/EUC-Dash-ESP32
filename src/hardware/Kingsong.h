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

#ifndef __KINGSONG_H
#define __KINGSONG_H

/*define constants for specific KS wheel models 
* add here when adding support for a new model
*/
#define KS_DEFAULT_MAXCURRENT   35 //maximum current draw
#define KS_DEFAULT_CRITTEMP     65 //critical internal temperature
#define KS_DEFAULT_WARNTEMP     50 //Hight internal temperature
#define KS_DEFAULT_BATTVOLT     67 //battery pack max voltage
#define KS_DEFAULT_BATTWARN     40 //remaining battery percentage warning
#define KS_DEFAULT_BATT_IR      30 //internal resistance in 1/100 ohms

#define KS_14D_MAXCURRENT   35 //maximum current draw
#define KS_14D_CRITTEMP     65 //critical internal temperature
#define KS_14D_WARNTEMP     50 //High internal temperature
#define KS_14D_BATTVOLT     67 //battery pack max voltage
#define KS_14D_BATTWARN     40 //remaining battery percentage warning
#define KS_14D_BATT_IR      30 //battery pack internal resistance in 1/100 ohms -- just a guess at the moment, will need tuning

#define KS_14S_MAXCURRENT   40
#define KS_14S_CRITTEMP     65
#define KS_14S_WARNTEMP     50
#define KS_14S_BATTVOLT     67
#define KS_14S_BATTWARN     25
#define KS_14S_BATT_IR      15

#define KS_16S_MAXCURRENT   40
#define KS_16S_CRITTEMP     65
#define KS_16S_WARNTEMP     50
#define KS_16S_BATTVOLT     67
#define KS_16S_BATTWARN     25
#define KS_16S_BATT_IR      15

#define KS_16X_MAXCURRENT   45
#define KS_16X_CRITTEMP     65
#define KS_16X_WARNTEMP     50
#define KS_16X_BATTVOLT     84
#define KS_16X_BATTWARN     25
#define KS_16X_BATT_IR      15


#include <TTGO.h>
#include "callback.h"

void ks_lights(byte mode);
void decodeKS(byte KSData[]);
void initks();

#endif /* __KINGSONG */