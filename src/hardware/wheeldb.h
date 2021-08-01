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

#ifndef _WHEELDB_H
    #define _WHEELDB_H

    #include <TTGO.h>
    
    enum { 
        LG_MJ1,         //18650 3500mAh 4.2V 46mOhm
        LG_MH1,         //18650 3200mAh 4.2V 55mOhm
        LG_M50T,        //21700 5000mAh 4.2V 31mOhm
        SAM_50G,        //21700 5000mAh 4.2V 23mOhm
        PANA_GA,        //18650 3450mAh 3.7(4.2V) 43mOhm
        PANA_BD,        //18650 3200mAh 3.7(4.2V) 45mOhm
        GENERIC_2900,   //18650 2900mAh 3.7(4.2V) 50mOhm
        GENERIC_2800,   //18650 2800mAh 3.7(4.2V) 50mOhm
        CELL_TYPE_NUM   //number of battry types
    };
    /**
     * @brief battery cell data structure
     */
    typedef struct {
        byte capacity; //Ah * 10
        byte resistance; //mOhm
    } wheeldb_celldata_t;

    enum { 
        KS14D0,  //1P 16S GENERIC_2900 500w 20kmh
        KS14D1,  //2P 16S GENERIC_2900 800w 30kmh
        KS14D2,  //2P 16S LG_MJ1 800w 30kmh
        KS14D3,  //4P 16S GENERIC_2900 800w 30kmh
        KS14D4,  //4P 16S LG_MJ1 800w 30kmh
        KS16S1,  //4P 16S GENERIC_2900 1200w 35kmh
        KS16S2,  //4P 16S LG_MJ1 1200w 35kmh
        KS16S4,  //2P 16S LG_MJ1 1200w 30kmh
        KS16X1,  //3P 20S LG_MJ1 2200w 45kmh
        KS16X2,  //6P 20S LG_MJ1 2200w 50kmh
        KSS181,  //3P 20S LG_M50T 2200w 50kmh
        KS18L2,  //4P 20S LG_MJ1 2200w 50kmh
        KS18L4,  //6P 20S LG_MJ1 2200w 50kmh
        KSUNKN,  //4P 16S LG_MJ1 1200w 35kmh
        WHEEL_TYPE_NUM  //number of battry types
    };

    /**
     * @brief wheel data structure
     */
    typedef struct {
        String manufacturer; //King Sonng, Gotway, Inmotion or Veteran
        String model;   //The wheel model eg. "KS16X" or "Nikola+"
        String mfgwh;   //Manufacturer stated battery capacity
        byte celltype;  //LG_MJ1, LG_MH1, LG_M50T, SAM_50G, PANA_GA,PANA_BD, GENERIC_2900 or GENERIC_2800
        byte numcellsP; //Number of cells in parallell
        byte numcellsS; //Number of cells in series
        byte maxspeed;  //kmh
        byte battwarn;  //at what battery percentage is speed throttled
        byte wheelsize; //in inches
        byte power;     //kw * 10 -- 1kw = 10
    } wheeldb_wheeldata_t;

    void wheeldb_setup(void);
    void wheeldb_set_wheelctl_data(byte wheel_type);

#endif