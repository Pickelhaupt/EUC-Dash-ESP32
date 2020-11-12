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

#include "config.h"
#include "Arduino.h"
#include "wheelctl.h"
#include "blectl.h"
#include "motor.h"
#include "callback.h"
#include "json_psram_allocator.h"
#include "alloc.h"

lv_task_t *speed_shake = nullptr;
lv_task_t *current_shake = nullptr;
lv_task_t *temp_shake = nullptr;

static void lv_speed_shake(lv_task_t *speed_shake);
static void lv_current_shake(lv_task_t *current_shake);
static void lv_temp_shake(lv_task_t *temp_shake);
void update_speed_shake(void);
void update_current_shake(void);
void update_temp_shake(void);

void wheelctl_update_max_min(int entry, float value, bool update_min);
void wheelctl_update_regen_current(int entry, float value);
void wheelctl_update_battpct_max_min(int entry, float value);
void update_calc_battery(float value);
void wheelctl_calc_power(void);

bool shakeoff[3] = {true, true, true};

wheelctl_data_t wheelctl_data[WHEELCTL_DATA_NUM];
wheelctl_constants_t wheelctl_constants[WHEELCTL_CONST_NUM];

void wheelctl_setup( void ){
    wheelctl_data[WHEELCTL_SPEED].max_value = 0;
    wheelctl_data[WHEELCTL_SPEED].min_value = 0;
    wheelctl_data[WHEELCTL_BATTPCT].max_value = 0;
    wheelctl_data[WHEELCTL_BATTPCT].min_value = 100;
    wheelctl_data[WHEELCTL_CURRENT].max_value = 0;
    wheelctl_data[WHEELCTL_CURRENT].min_value = 0;
    wheelctl_data[WHEELCTL_TEMP].min_value = 0;
}

float wheelctl_get_data(int entry)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        return (wheelctl_data[entry].value);
    }
    return 0;
}

void wheelctl_set_data(int entry, float value)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        wheelctl_data[entry].value = value;
        /* debug
        Serial.print("Wheeldata entry: ");
        Serial.print(entry);
        Serial.print(" value: ");
        Serial.println(value);
        */
        switch (entry)
        {
        case WHEELCTL_VOLTAGE:
            wheelctl_update_max_min(entry, value, true);
            update_calc_battery(value);
            break;
        case WHEELCTL_SPEED:
            update_current_shake();
            wheelctl_update_max_min(entry, value, false);
            break;
        case WHEELCTL_CURRENT:
            update_current_shake();
            wheelctl_update_max_min(entry, value, false);
            wheelctl_update_regen_current(entry, value);
            wheelctl_calc_power();
            break;
        case WHEELCTL_TEMP:
            update_temp_shake();
            wheelctl_update_max_min(entry, value, true);
            break;
        case WHEELCTL_BATTPCT:
            wheelctl_update_battpct_max_min(entry, value);
            break;
        }
    }
}

void wheelctl_update_max_min(int entry, float value, bool update_min)
{
    if (wheelctl_data[entry].value > wheelctl_data[entry].max_value)
    {
        wheelctl_data[entry].max_value = wheelctl_data[entry].value;
    }
    if (wheelctl_data[entry].value < wheelctl_data[entry].min_value && update_min)
    {
        wheelctl_data[entry].min_value = wheelctl_data[entry].value;
    }
}

void wheelctl_update_battpct_max_min(int entry, float value){
    if (wheelctl_data[WHEELCTL_CURRENT].value >= 0) {
        wheelctl_update_max_min(entry, value, true);
    }
    else {
        wheelctl_update_max_min(entry, value, false);
    }
}

void wheelctl_update_regen_current(int entry, float value) {
        if (value < 0)
    {
        float negamp = (value * -1);
        if (negamp > wheelctl_data[entry].min_value)
        {
            wheelctl_data[entry].min_value = negamp;
        }
    }
}

void wheelctl_calc_power(void) {
    wheelctl_set_data(WHEELCTL_POWER, wheelctl_data[WHEELCTL_CURRENT].value * wheelctl_data[WHEELCTL_VOLTAGE].value);
}

void update_calc_battery(float value)
{
    int centivolt = value * 100;
    if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 70)
    {
        if (centivolt > 6680) wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 5440) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 5320) / 13.6);
        else if (centivolt > 5120) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 5120) / 36.0);
        else wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
        return;
    }
    else if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 86) 
    {
        if (centivolt > 8350) wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 6810) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 6650) / 13.6);
        else if (centivolt > 6420) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 6400) / 36.0);
        else wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
        return;
    }
    else if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 105) 
    {
        if (centivolt > 9986) wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 8132) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 8040) / 13.6);
        else if (centivolt > 7660) wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 7660) / 36.0);
        else wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
        return;
    }
}

float wheelctl_get_max_data(int entry)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        return (wheelctl_data[entry].max_value);
    }
    return 0;
}

void wheelctl_set_max_data(int entry, float max_value)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        wheelctl_data[entry].max_value = max_value;
    }
}

float wheelctl_get_min_data(int entry)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        return (wheelctl_data[entry].min_value);
    }
    return 0;
}

void wheelctl_set_min_data(int entry, float min_value)
{
    if (entry < WHEELCTL_DATA_NUM)
    {
        wheelctl_data[entry].min_value = min_value;
    }
}

byte wheelctl_get_constant(int entry)
{
    if (entry < WHEELCTL_CONST_NUM)
    {
        return (wheelctl_constants[entry].value);
    }
    return 0;
}

void wheelctl_set_constant(int entry, byte value)
{
    if (entry < WHEELCTL_CONST_NUM)
    {
        wheelctl_constants[entry].value = value;
    }
}

//Haptic feedback
void update_speed_shake(void)
{
    if (wheelctl_data[WHEELCTL_SPEED].value >= wheelctl_data[WHEELCTL_ALARM3].value && shakeoff[0])
    {
        speed_shake = lv_task_create(lv_speed_shake, 750, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(speed_shake);
        shakeoff[0] = false;
    }
    else if (wheelctl_data[WHEELCTL_SPEED].value < wheelctl_data[WHEELCTL_ALARM3].value && !shakeoff[0] && speed_shake != nullptr)
    {
        lv_task_del(speed_shake);
        shakeoff[0] = true;
    }
}

void update_current_shake(void)
{
    if (wheelctl_data[WHEELCTL_CURRENT].value >= (wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value * 0.75) && shakeoff[1] && current_shake != nullptr)
    {
        current_shake = lv_task_create(lv_current_shake, 200, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(current_shake);
        shakeoff[1] = false;
    }
    else if (wheelctl_data[WHEELCTL_CURRENT].value < (wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value * 0.75) && !shakeoff[1])
    {
        lv_task_del(current_shake);
        shakeoff[1] = true;
    }
}

void update_temp_shake(void)
{
    if (wheelctl_data[WHEELCTL_TEMP].value > wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value && shakeoff[2])
    {
        temp_shake = lv_task_create(lv_temp_shake, 1000, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(temp_shake);
        shakeoff[2] = false;
    }
    else if (wheelctl_data[WHEELCTL_TEMP].value <= wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value && !shakeoff[2] && temp_shake != nullptr)
    {
        lv_task_del(temp_shake);
        shakeoff[2] = true;
    }
}

static void lv_current_shake(lv_task_t *current_shake)
{
    motor_vibe(30, true);
}

static void lv_temp_shake(lv_task_t *temp_shake)
{
    motor_vibe(30, true);
    delay(250);
    motor_vibe(30, true);
}

static void lv_speed_shake(lv_task_t *speed_shake)
{
    motor_vibe(30, true);
} //End haptic feedback