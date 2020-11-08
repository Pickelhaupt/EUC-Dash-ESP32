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
#define KS_14D_MAXCURRENT   35
#define KS_14D_CRITTEMP     65
#define KS_14D_WARNTEMP     50
#define KS_14D_BATTVOLT     67
#define KS_14D_BATTWARN     40

#define KS_14S_MAXCURRENT   35
#define KS_14S_CRITTEMP     65
#define KS_14S_WARNTEMP     50
#define KS_14S_BATTVOLT     67
#define KS_14S_BATTWARN     25

#define KS_16S_MAXCURRENT   40
#define KS_16S_CRITTEMP     65
#define KS_16S_WARNTEMP     50
#define KS_16S_BATTVOLT     67
#define KS_16S_BATTWARN     25

#define KS_16X_MAXCURRENT   45
#define KS_16X_CRITTEMP     65
#define KS_16X_WARNTEMP     50
#define KS_16X_BATTVOLT     84
#define KS_16X_BATTWARN     25


#include <TTGO.h>
#include "callback.h"

struct Wheel_constants
{
    byte maxcurrent;
    byte crittemp;
    byte warntemp;
    byte battvolt;
    byte battwarn;
};
extern struct Wheel_constants wheelconst;
void decodeKS(byte KSData[]);
void initks();

extern float wheeldata[];

#endif /* __KINGSONG */