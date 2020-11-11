/****************************************************************************
 * 2020 Jesper Ortlund
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

#ifndef _WHEELCTL_H
    #define _WHEELCTL_H

    #include <TTGO.h>
    #include "callback.h"

    #define WHEELCTL_JSON_CONFIG_FILE    "/wheelctl.json" /** @brief defines json config file name */

    /**
     * @brief wheel config structure
     */
    typedef struct {
        bool enable=true;
    } wheelctl_config_t;

    enum { 
        WHEELCTL_CONFIG_LIGHTS,        //enable long press on dashboard to toggle lights
        WHEELCTL_CONFIG_NUM     //number of wheel configuration parameters
    };

    /**
     * @brief wheel constants structure
     */
    typedef struct {
        byte value;
    } wheelctl_constants_t;

    enum { 
        WHEELCTL_CONST_MAXCURRENT,  //Maximum current draw for wheel model
        WHEELCTL_CONST_CRITTEMP,    //Critical internal temperature
        WHEELCTL_CONST_WARNTEMP,    //internal temperature to trigger warning
        WHEELCTL_CONST_BATTVOLT,    //Voltage of the battery pack for the wheel model
        WHEELCTL_CONST_BATTWARN,    //Percentage of battery remaining when warning should be triggered for the specific wheel model        
        WHEELCTL_CONST_NUM          //number of wheel constants
    };

    /** This wastes 80 bytes due to storing unused max and min floats for entries that doesn't need them
     * However, at the moment I think it's a worthwile tradeoff
     * @brief wheel data structure, holds current, max and min values
     */
    typedef struct {
        float value = 0;
        float max_value = 0;
        float min_value = 0;
    } wheelctl_data_t;

    enum { 
        WHEELCTL_VOLTAGE,   //Current battery voltage
        WHEELCTL_SPEED,     //Current speed in kmh
        WHEELCTL_ODO,       //Total ride distance since new
        WHEELCTL_CURRENT,   //Current current consumption
        WHEELCTL_TEMP,      //Internal wheel temperature
        WHEELCTL_RMODE,     //Wheel ride mode
        WHEELCTL_BATTPCT,   //Calculated percentage of remaining battery
        WHEELCTL_POWER,     //Wheel power output
        WHEELCTL_TRIP,      //Ride distance since power on
        WHEELCTL_UPTIME,    //Time since power on in seconds
        WHEELCTL_TOPSPEED,  //max speed reached in kmh since power on
        WHEELCTL_FANSTATE,  //Internal fan run state
        WHEELCTL_ALARM1,    //Speed when first alarm is triggered
        WHEELCTL_ALARM2,    //Speed when second alarm is triggered
        WHEELCTL_ALARM3,    //Speed when third alarm is triggered
        WHEELCTL_TILTBACK,  //Speed when tiltback is triggered
        WHEELCTL_RIDETIME,  //Total time in motion since power on
        WHEELCTL_DATA_NUM   //number of data entries
    };


    /**
     * @brief get the value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_ODO, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP, WHEELCTL_RMODE, WHEELCTL_BATTPCT, WHEELCTL_POWER, WHEELCTL_TRIP, WHEELCTL_UPTIME, 
     * WHEELCTL_TOPSPEED, WHEELCTL_FANSTATE, WHEELCTL_ALARM1, WHEELCTL_ALARM2, WHEELCTL_ALARM3, WHEELCTL_TILTBACK, 
     * WHEELCTL_RIDETIME, WHEELCTL_DATA_NUM
     * 
     * @return  float
     */
    float wheelctl_get_data( int entry );

    /**
     * @brief set the value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_ODO, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP, WHEELCTL_RMODE, WHEELCTL_BATTPCT, WHEELCTL_POWER, WHEELCTL_TRIP, WHEELCTL_UPTIME, 
     * WHEELCTL_TOPSPEED, WHEELCTL_FANSTATE, WHEELCTL_ALARM1, WHEELCTL_ALARM2, WHEELCTL_ALARM3, WHEELCTL_TILTBACK, 
     * WHEELCTL_RIDETIME
     * @param   value    the value of the data entry
     */
    void wheelctl_set_data( int entry, float value );

    /**
     * @brief get the max value for a specific wheel data entry
     * 
     * @param   entry     onfigitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP,  WHEELCTL_BATTPCT, WHEELCTL_POWER
     * 
     * @return  float
     */
    float wheelctl_get_max_data( int entry );

    /**
     * @brief set the max value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP,  WHEELCTL_BATTPCT, WHEELCTL_POWER
     * @param   max_value    the value of the data entry
     */
    void wheelctl_set_max_data( int entry, float max_value );

    /**
     * @brief get the min value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP,  WHEELCTL_BATTPCT, WHEELCTL_POWER
     * 
     * @return  float
     */
    float wheelctl_get_min_data( int entry );

    /**
     * @brief set the min value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_VOLTAGE, WHEELCTL_SPEED, WHEELCTL_CURRENT,  
     * WHEELCTL_TEMP,  WHEELCTL_BATTPCT, WHEELCTL_POWER
     * @param   min_value    the value of the data entry
     */
    void wheelctl_set_min_data( int entry, float min_value );

    /**
     * @brief get the  value of the wheel constant
     * 
     * @param   entry     configitem: WHEELCTL_CONST_MAXCURRENT,  WHEELCTL_CONST_CRITTEMP, WHEELCTL_CONST_WARNTEMP,   WHEELCTL_CONST_BATTVOLT, WHEELCTL_CONST_BATTWARN
     * 
     * @return  uint8_t
     */
    byte wheelctl_get_constant( int entry );

    /**
     * @brief set the min value for a specific wheel data entry
     * 
     * @param   entry     configitem: WHEELCTL_CONST_MAXCURRENT,  WHEELCTL_CONST_CRITTEMP, WHEELCTL_CONST_WARNTEMP,   WHEELCTL_CONST_BATTVOLT, WHEELCTL_CONST_BATTWARN
     * @param   value     the value of the constant
     */
    void wheelctl_set_constant( int entry, uint8_t value );


    #endif