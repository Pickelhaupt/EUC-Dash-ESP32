/****************************************************************************
 * 2021 Jesper Ortlund
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

#include "wheeldb.h"
#include "wheelctl.h"
#include <ttgo.h>
#include "Arduino.h"

wheeldb_celldata_t celldata[CELL_TYPE_NUM];
wheeldb_wheeldata_t wheeldata[WHEEL_TYPE_NUM];

void wheeldb_set_celldata(void){
    celldata[LG_MJ1] = {35, 46};
    celldata[LG_MH1] = {32, 55};
    celldata[LG_M50T] = {50, 31};
    celldata[SAM_50G] = {50, 23};
    celldata[PANA_GA] = {34, 43};
    celldata[PANA_BD] = {32, 45};
    celldata[GENERIC_2900] = {29, 50};
    celldata[GENERIC_2800] = {28, 50};
}

void wheeldb_set_wheeldata(void) {

    //KingSong
    //                   Mfg    Model    wh    Battery type  P  S  spd thr dia  w
    wheeldata[KS14D0] = {"KS", "KS14M", "174", GENERIC_2900, 1, 16, 20, 50, 14, 5};
    wheeldata[KS14D1] = {"KS", "KS14D", "340", GENERIC_2900, 2, 16, 30, 50, 14, 8};
    wheeldata[KS14D2] = {"KS", "KS14D", "420", LG_MJ1, 2, 16, 30, 50, 14, 8};
    wheeldata[KS14D3] = {"KS", "KS14S", "680", GENERIC_2900, 4, 16, 30, 30, 14, 8};
    wheeldata[KS14D4] = {"KS", "KS14S", "840", LG_MJ1, 4, 16, 30, 30, 14, 8};
    wheeldata[KS16S1] = {"KS", "KS16S", "680", GENERIC_2900, 4, 16, 35, 30, 16, 12};
    wheeldata[KS16S2] = {"KS", "KS16S", "840", LG_MJ1, 4, 16, 30, 50, 16, 12};
    wheeldata[KS16S4] = {"KS", "KS16S", "420", LG_MJ1, 2, 16, 35, 30, 16, 12};
    wheeldata[KS16X1] = {"KS", "KS16XS", "777", LG_MJ1, 3, 20, 45, 40, 16, 22};
    wheeldata[KS16X2] = {"KS", "KS16X", "1554", LG_MJ1, 6, 20, 50, 30, 16, 22};
    wheeldata[KSS181] = {"KS", "KSS18", "1110", LG_M50T, 3, 20, 50, 30, 18, 22};
    wheeldata[KS18L2] = {"KS", "KS18L", "1036", LG_MJ1, 4, 20, 50, 30, 18, 22};
    wheeldata[KS18L4] = {"KS", "KS18XL", "1554", LG_MJ1, 6, 20, 50, 30, 18, 22};
    wheeldata[KSUNKN] = {"KS", "Unknown", "840", LG_MJ1, 4, 16, 45, 30, 16, 12};

    //Gotway

    //Inmotion

    //Veteran
}

void wheeldb_set_wheelctl_data(byte wheel_type) {
    wheelctl_set_info(WHEELCTL_INFO_MANUFACTURER, wheeldata[wheel_type].manufacturer);
    wheelctl_set_info(WHEELCTL_INFO_MODEL, wheeldata[wheel_type].model);
    wheelctl_set_info(WHEELCTL_INFO_BATTCAP, wheeldata[wheel_type].mfgwh);
    wheelctl_set_constant(WHEELCTL_CONST_MAXCURRENT, (200 * wheeldata[wheel_type].power / (wheeldata[wheel_type].numcellsS * 3.7)));
    wheelctl_set_constant(WHEELCTL_CONST_BATTVOLT, (wheeldata[wheel_type].numcellsS * 4.2));
    wheelctl_set_constant(WHEELCTL_CONST_BATTWARN, wheeldata[wheel_type].battwarn);
    wheelctl_set_constant(WHEELCTL_CONST_BATT_IR, (celldata[wheeldata[wheel_type].celltype].resistance * wheeldata[wheel_type].numcellsS / (10 * wheeldata[wheel_type].numcellsP)));
}

void wheeldb_setup(void){
    wheeldb_set_celldata();
    wheeldb_set_wheeldata();
}