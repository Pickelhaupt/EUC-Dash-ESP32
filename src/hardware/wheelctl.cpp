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
//#include "callback.h"
#include "json_psram_allocator.h"
//#include "alloc.h"
#include "Kingsong.h"
#include "powermgm.h"
#include "gui/mainbar/fulldash_tile/fulldash_tile.h"
#include "gui/mainbar/simpledash_tile/simpledash_tile.h"

#define BATT_AVG_ENTRIES 10

lv_task_t *speed_shake = nullptr;
lv_task_t *current_shake = nullptr;
lv_task_t *temp_shake = nullptr;
lv_task_t *ride_tick = nullptr;
lv_task_t *save_trip_task = nullptr;

static void lv_speed_shake(lv_task_t *speed_shake);
static void lv_current_shake(lv_task_t *current_shake);
static void lv_temp_shake(lv_task_t *temp_shake);
void update_speed_shake(float value);
void update_current_shake(float value);
void update_temp_shake(float value);

void wheelctl_update_max_min(int entry, float value, bool update_min);
void wheelctl_update_regen_current(int entry, float value);
void wheelctl_update_battpct_max_min(int entry, float value);
void update_calc_battery(float value);
void wheelctl_calc_power(float value);
void wheelctl_minute_update(lv_task_t *ride_tick);
void wheelctl_save_trip_task(lv_task_t *save_trip_task);
void wheelctl_update_powercons( void );
void wheelctl_update_avgspeed(float value);
void wheelctl_update_watch_trip(float value);
void wheelctl_read_config( void );
void current_trip_read_data(void);
void current_trip_save_data(void);


bool shakeoff[3] = {true, true, true};
bool lightsoff = true;
bool firstrun[WHEELCTL_DATA_NUM];
float old_uptime = 0;
bool newtrip = true;
bool sync_trip = true;

wheelctl_data_t wheelctl_data[WHEELCTL_DATA_NUM];
wheelctl_constants_t wheelctl_constants[WHEELCTL_CONST_NUM];
wheelctl_info_t wheelctl_info[WHEELCTL_INFO_NUM];
wheelctl_config_t wheelctl_config[WHEELCTL_CONFIG_NUM];
trip_data_t current_trip;

/**
* @brief set wheel constants to initial default values
*/
void wheelctl_init_constants( void ) {
    wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value = 40;
    wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value = 65;
    wheelctl_constants[WHEELCTL_CONST_WARNTEMP].value = 55;
    wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value = 67;
    wheelctl_constants[WHEELCTL_CONST_BATTWARN].value = 40;
    wheelctl_constants[WHEELCTL_CONST_BATTCRIT].value = 10;
    wheelctl_constants[WHEELCTL_CONST_BATT_IR].value = 20;
}

void wheelctl_setup(void)
{
    wheelctl_data[WHEELCTL_SPEED].max_value = 0;
    wheelctl_data[WHEELCTL_SPEED].min_value = 0;
    wheelctl_data[WHEELCTL_BATTPCT].max_value = 0;
    wheelctl_data[WHEELCTL_BATTPCT].min_value = 100;
    wheelctl_data[WHEELCTL_CURRENT].max_value = 0;
    wheelctl_data[WHEELCTL_CURRENT].min_value = 0;
    wheelctl_data[WHEELCTL_TEMP].min_value = 0;
    wheelctl_init_constants();
    wheelctl_update_values();
    wheelctl_read_config();
    current_trip_read_data();
    
    motor_vibe(5, true);
}

void wheelctl_connect_actions(void) 
{
    if (wheelctl_config[WHEELCTL_CONFIG_LIGHTS_OFF].enable) {
        lightsoff = true;
        wheelctl_toggle_lights();
    }
    sync_trip = true;
    ride_tick = lv_task_create( wheelctl_minute_update, 1000, LV_TASK_PRIO_LOW, NULL );
    save_trip_task = lv_task_create( wheelctl_save_trip_task, 20000, LV_TASK_PRIO_LOW, NULL );
}

void wheelctl_disconnect_actions(){
    if (ride_tick != nullptr) lv_task_del(ride_tick);
    if (save_trip_task != nullptr) lv_task_del(save_trip_task);
}

void wheelctl_update_values(void)
{
    for (int i = 0; i < WHEELCTL_DATA_NUM; i++)
    {
        firstrun[i] = {true};
    }
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
        if (firstrun[entry]) wheelctl_data[entry].value = value;
        switch (entry)
        {
        case WHEELCTL_SPEED:
            if (wheelctl_data[entry].value != value || firstrun[entry])
            {
                update_speed_shake(value);
                wheelctl_update_max_min(entry, value, false);
                if (fulldash_active) fulldash_speed_update(value, wheelctl_data[WHEELCTL_ALARM3].value, wheelctl_data[WHEELCTL_TILTBACK].value, wheelctl_data[WHEELCTL_SPEED].max_value, wheelctl_data[WHEELCTL_SPEED].min_value);
                if (simpledash_active) simpledash_speed_update(value, wheelctl_data[WHEELCTL_ALARM3].value, wheelctl_data[WHEELCTL_TILTBACK].value, wheelctl_data[WHEELCTL_SPEED].max_value);
                firstrun[entry] = false;
            }
            break;
        case WHEELCTL_CURRENT:
            if (wheelctl_data[entry].value != value || firstrun[entry])
            {
                update_current_shake(value);
                wheelctl_update_max_min(entry, value, false);
                wheelctl_update_regen_current(entry, value);
                wheelctl_calc_power(value);
                if (fulldash_active) fulldash_current_update(value, wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value, wheelctl_data[WHEELCTL_CURRENT].min_value, wheelctl_data[WHEELCTL_CURRENT].max_value);
                if (simpledash_active) simpledash_current_update(value, wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value, wheelctl_data[WHEELCTL_CURRENT].min_value, wheelctl_data[WHEELCTL_CURRENT].max_value);
                firstrun[entry] = false;
            }
            break;
        case WHEELCTL_VOLTAGE:
            if (wheelctl_data[entry].value != value || firstrun[entry])
            {
                wheelctl_update_max_min(entry, value, true);
                update_calc_battery(value);
                firstrun[entry] = false;
            }
            break;
        case WHEELCTL_TEMP:
            if (wheelctl_data[entry].value != value || firstrun[entry])
            {
                update_temp_shake(value);
                wheelctl_update_max_min(entry, value, true);
                if (fulldash_active) fulldash_temp_update(value, wheelctl_constants[WHEELCTL_CONST_WARNTEMP].value, wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value, wheelctl_data[WHEELCTL_TEMP].max_value);
                firstrun[entry] = false;
            }
            break;
        case WHEELCTL_BATTPCT:
            if (wheelctl_data[entry].value != value || firstrun[entry])
            {
                wheelctl_update_battpct_max_min(entry, value);
                if (fulldash_active) fulldash_batt_update(value, wheelctl_data[WHEELCTL_BATTPCT].min_value, wheelctl_data[WHEELCTL_BATTPCT].max_value);
                if (simpledash_active) simpledash_batt_update(value, wheelctl_data[WHEELCTL_BATTPCT].min_value, wheelctl_data[WHEELCTL_BATTPCT].max_value);
                firstrun[entry] = false;
            }
            break;
        case WHEELCTL_POWER:
            wheelctl_update_max_min(entry, value, false);
            break;
        case WHEELCTL_TRIP:
            wheelctl_update_watch_trip(value);
            wheelctl_update_avgspeed(value);
            break;
        case WHEELCTL_FANSTATE:
            if (fulldash_active) fulldash_fan_indic(value);
            if (simpledash_active) simpledash_fan_indic(value);
            break;
        }
        firstrun[entry] = false;
        wheelctl_data[entry].value = value;
    }
}

void wheelctl_minute_update(lv_task_t *ride_tick)
{
    if (wheelctl_data[WHEELCTL_SPEED].value >= MIN_RIDE_SPEED) {
        wheelctl_data[WHEELCTL_RIDETIME].value++;
        current_trip.ride_time++;
    }
    
    wheelctl_update_powercons();
    current_trip.max_speed = wheelctl_data[WHEELCTL_SPEED].max_value;
    current_trip.max_current = wheelctl_data[WHEELCTL_CURRENT].max_value;
    current_trip.max_power = wheelctl_data[WHEELCTL_POWER].max_value;
    current_trip.max_temperature = wheelctl_data[WHEELCTL_TEMP].max_value;
}

void wheelctl_save_trip_task(lv_task_t *save_trip_task) {
    current_trip_save_data();
}

void wheelctl_update_watch_trip(float value)
{
    static bool last_value_set = false;
    static float last_value;
    if (!last_value_set || value < last_value || sync_trip) { 
        last_value = value; 
        last_value_set = true;
    }
    wheelctl_data[WHEELCTL_TRIP].max_value += (value - last_value);
    current_trip.trip += (value - last_value);
    last_value = value;
    sync_trip = false;
}

void wheelctl_update_avgspeed(float value)
{
    float trip_distance = wheelctl_data[WHEELCTL_TRIP].max_value;
    if (wheelctl_data[WHEELCTL_RIDETIME].value != 0) {
        wheelctl_data[WHEELCTL_SPEED].min_value = trip_distance / (wheelctl_data[WHEELCTL_RIDETIME].value / 3600);
    }
    current_trip.avg_speed = wheelctl_data[WHEELCTL_SPEED].min_value;
}

void wheelctl_update_powercons() {
    float trip_distance = wheelctl_data[WHEELCTL_TRIP].max_value;
    wheelctl_data[WHEELCTL_POWERCONS].value = wheelctl_data[WHEELCTL_POWERCONS].value + (wheelctl_data[WHEELCTL_POWER].value / 3600);
    if (trip_distance !=0) wheelctl_data[WHEELCTL_ECONOMY].value = wheelctl_data[WHEELCTL_POWERCONS].value / trip_distance;
    
    current_trip.consumed_energy = current_trip.consumed_energy + (wheelctl_data[WHEELCTL_POWER].value / 3600);
    if (current_trip.trip != 0) current_trip.trip_economy = current_trip.consumed_energy / current_trip.trip;
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

void wheelctl_update_battpct_max_min(int entry, float value)
{
    if (wheelctl_data[WHEELCTL_CURRENT].value > 0)
    {
        wheelctl_update_max_min(entry, value, true);
        current_trip.max_battery = wheelctl_data[WHEELCTL_BATTPCT].max_value;
        current_trip.min_battery = wheelctl_data[WHEELCTL_BATTPCT].min_value;
    }
    if (value < 10) {
        if (fulldash_active) fulldash_batt_alert(true);
        if (simpledash_active) simpledash_batt_alert(true);
    }
    else {
        if (fulldash_active) fulldash_batt_alert(false);
        if (simpledash_active) simpledash_batt_alert(false);
    }
}

void wheelctl_update_regen_current(int entry, float value)
{
    if (value < 0)
    {
        float negamp = (value * -1);
        if (negamp > wheelctl_data[entry].min_value)
        {
            wheelctl_data[entry].min_value = negamp;
            current_trip.max_regen_current = negamp;
        }
    }
}

void wheelctl_calc_power(float value)
{
    wheelctl_set_data(WHEELCTL_POWER, wheelctl_data[WHEELCTL_VOLTAGE].value * value);
}

void update_calc_battery(float value)
{
    float voltagesag = wheelctl_constants[WHEELCTL_CONST_BATT_IR].value * wheelctl_data[WHEELCTL_CURRENT].value;

    /*
    static int centivolt_array[BATT_AVG_ENTRIES] = {0};
    static int i = 0;
    int sum = 0;
    int num = 0;
    if (i < BATT_AVG_ENTRIES) i++; else i = 0;

    centivolt_array[i] = (value * 100) + voltagesag; //compensate for battery pack internal resitance
    for (int j = 0; j < BATT_AVG_ENTRIES; j++)
    {
        if (centivolt_array[j] > 0)
        {
            sum += centivolt_array[j];
            num++;
        }
    }
    int centivolt = sum / num;
    */

    int centivolt = (value * 100) + voltagesag;

    if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 70)
    {
        if (centivolt > 6680)
            wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 5440)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 5320) / 13.6);
        else if (centivolt > 5120)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 5120) / 36.0);
        else
            wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
        return;
    }
    else if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 86)
    {
        if (centivolt > 8350)
            wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 6800)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 6650) / 17);
        else if (centivolt > 6400)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 6400) / 45);
        else
            wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
        return;
    }
    else if (wheelctl_constants[WHEELCTL_CONST_BATTVOLT].value < 105)
    {
        if (centivolt > 10020)
            wheelctl_set_data(WHEELCTL_BATTPCT, 100.0);
        else if (centivolt > 8160)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 8070) / 19.5);
        else if (centivolt > 7660)
            wheelctl_set_data(WHEELCTL_BATTPCT, (centivolt - 7935) / 48.75);
        else
            wheelctl_set_data(WHEELCTL_BATTPCT, 0.0);
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

String wheelctl_get_info(int entry)
{
    if (entry < WHEELCTL_INFO_NUM)
    {
        return (wheelctl_info[entry].value);
    }
    return "undefined";
}

void wheelctl_set_info(int entry, String value)
{
    if (entry < WHEELCTL_INFO_NUM)
    {
        wheelctl_info[entry].value = value;
    }
}

//Haptic feedback
void update_speed_shake(float value)
{
    if (value >= wheelctl_data[WHEELCTL_ALARM3].value && shakeoff[0])
    {
        powermgm_set_event(POWERMGM_BMA_TILT);
        speed_shake = lv_task_create(lv_speed_shake, 250, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(speed_shake);
        shakeoff[0] = false;
        log_i("speedshake on");
    }
    else if (value < wheelctl_data[WHEELCTL_ALARM3].value && !shakeoff[0] && speed_shake != nullptr)
    {
        lv_task_del(speed_shake);
        shakeoff[0] = true;
        log_i("speedshake off");
    }
}

void update_current_shake(float value)
{
    if (value >= (wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value * 0.75) && shakeoff[1] && current_shake != nullptr)
    {
        powermgm_set_event(POWERMGM_BMA_TILT);
        current_shake = lv_task_create(lv_current_shake, 500, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(current_shake);
        shakeoff[1] = false;
        if (fulldash_active) fulldash_current_alert(true);
        if (simpledash_active) simpledash_current_alert(true);
    }
    else if (value < (wheelctl_constants[WHEELCTL_CONST_MAXCURRENT].value * 0.75) && !shakeoff[1])
    {
        lv_task_del(current_shake);
        shakeoff[1] = true;
        if (fulldash_active) fulldash_current_alert(false);
        if (simpledash_active) simpledash_current_alert(false);
    }
}

void update_temp_shake(float value)
{
    if (value > wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value && shakeoff[2])
    {
        powermgm_set_event(POWERMGM_BMA_TILT);
        temp_shake = lv_task_create(lv_temp_shake, 1000, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(temp_shake);
        shakeoff[2] = false;
        if (fulldash_active) fulldash_temp_alert(true);
        if (simpledash_active) simpledash_temp_alert(true);
    }
    else if (value <= wheelctl_constants[WHEELCTL_CONST_CRITTEMP].value && !shakeoff[2] && temp_shake != nullptr)
    {
        lv_task_del(temp_shake);
        shakeoff[2] = true;
        if (fulldash_active) fulldash_temp_alert(false);
        if (simpledash_active) simpledash_temp_alert(false);
    }
}

static void lv_current_shake(lv_task_t *current_shake)
{
    motor_vibe(50, true);
}

static void lv_temp_shake(lv_task_t *temp_shake)
{
    motor_vibe(20, true);
    delay(150);
    motor_vibe(20, true);
}

static void lv_speed_shake(lv_task_t *speed_shake)
{
    motor_vibe(20, true);
} //End haptic feedback

void wheelctl_toggle_lights(void)
{
    String wheeltype = wheelctl_info[WHEELCTL_INFO_MANUFACTURER].value;
    if (wheeltype == "KS")
    {
        if (blectl_cli_getconnected()) {
            ks_lights(lightsoff);
            if (wheelctl_config[WHEELCTL_CONFIG_LED].enable) ks_led(lightsoff);
        }
        if (lightsoff) lightsoff = false; else lightsoff = true;
    }
    else if (wheeltype == "GW")
    {
        //if (blectl_cli_getconnected()) gw_lights(lightsoff);
        //if (lightsoff) lightsoff = false; else lightsoff = true;
    }
    else if (wheeltype == "IM")
    {
        //if (blectl_cli_getconnected()) im_lights(lightsoff);
        //if (lightsoff) lightsoff = false; else lightsoff = true;
    }
    else if (wheeltype == "NBZ")
    {
        //if (blectl_cli_getconnected()) nbz_lights(lightsoff);
        //if (lightsoff) lightsoff = false; else lightsoff = true;
    }
    else if (wheeltype == "NB")
    {
        //if (blectl_cli_getconnected()) nb_lights(lightsoff);
        //if (lightsoff) lightsoff = false; else lightsoff = true;
    }
}

void wheelctl_save_config( void ) {
    fs::File file = SPIFFS.open( WHEELCTL_JSON_CONFIG_FILE, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", WHEELCTL_JSON_CONFIG_FILE );
    }
    else {
        SpiRamJsonDocument doc( 1000 );

        doc["lightsoff"] = wheelctl_config[ WHEELCTL_CONFIG_LIGHTS_OFF ].enable;
        doc["led"] = wheelctl_config[ WHEELCTL_CONFIG_LED ].enable;
        doc["horn"] = wheelctl_config[ WHEELCTL_CONFIG_HORN ].enable;
        doc["simple"] = wheelctl_config[ WHEELCTL_CONFIG_HAPTIC ].enable;
        if ( serializeJsonPretty( doc, file ) == 0) {
            log_e("Failed to write config file");
        }
        doc.clear();
    }
    file.close();
}

void wheelctl_read_config( void ) {
    fs::File file = SPIFFS.open( WHEELCTL_JSON_CONFIG_FILE, FILE_READ );
    if (!file) {
        log_e("Can't open file: %s!", WHEELCTL_JSON_CONFIG_FILE );
    }
    else {
        int filesize = file.size();
        SpiRamJsonDocument doc( filesize * 2 );

        DeserializationError error = deserializeJson( doc, file );
        if ( error ) {
            log_e("update check deserializeJson() failed: %s", error.c_str() );
        }
        else {
            wheelctl_config[ WHEELCTL_CONFIG_LIGHTS_OFF ].enable = doc["lightsoff"] | true;
            wheelctl_config[ WHEELCTL_CONFIG_LED ].enable = doc["led"] | true;
            wheelctl_config[ WHEELCTL_CONFIG_HORN ].enable = doc["horn"] | false;
            wheelctl_config[ WHEELCTL_CONFIG_HAPTIC ].enable = doc["haptic"] | false;
        }        
        doc.clear();
    }
    file.close();
}

void current_trip_save_data(void)
{
    fs::File file = SPIFFS.open(CURRENT_TRIP_JSON_FILE, FILE_WRITE);

    if (!file)
    {
        log_e("Can't open file: %s!", CURRENT_TRIP_JSON_FILE);
    }
    else
    {
        log_e("Saving trip data file %s", CURRENT_TRIP_JSON_FILE);
        SpiRamJsonDocument doc(1500);

        doc["timestamp"] = current_trip.timestamp;
        doc["ride_time"] = current_trip.ride_time;
        doc["trip"] = current_trip.trip;
        doc["max_speed"] = current_trip.max_speed;
        doc["avg_speed"] = current_trip.avg_speed;
        doc["max_current"] = current_trip.max_current;
        doc["max_regen_current"] = current_trip.max_regen_current;
        doc["max_power"] = current_trip.max_power;
        doc["max_battery"] = current_trip.max_battery;
        doc["min_battery"] = current_trip.min_battery;
        doc["max_temperature"] =  current_trip.max_temperature;
        doc["cunsumed_energy"] = current_trip.consumed_energy;
        doc["trip_economy"] = current_trip.trip_economy;

        if (serializeJsonPretty(doc, file) == 0)
        {
            log_e("Failed to write config file");
        }
        doc.clear();
    }
    file.close();
}

void current_trip_read_data(void)
{
    fs::File file = SPIFFS.open(CURRENT_TRIP_JSON_FILE, FILE_READ);

    if (!file)
    {
        log_e("Can't open file: %s!", CURRENT_TRIP_JSON_FILE);
    }
    else
    {
        int filesize = file.size();
        SpiRamJsonDocument doc(filesize * 2);

        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            log_e("blectl deserializeJson() failed: %s", error.c_str());
        }
        else
        {
            current_trip.timestamp = doc["timestamp"] | 0;
            current_trip.ride_time = doc["ride_time"] | 0.0;
            current_trip.trip = doc["trip"] | 0.0;
            current_trip.max_speed = doc["max_speed"] | 0.0;
            current_trip.avg_speed = doc["avg_speed"] | 0.0;
            current_trip.max_current = doc["max_current"] | 0.0;
            current_trip.max_regen_current = doc["max_regen_current"] | 0.0;
            current_trip.max_power = doc["max_power"] | 0.0;
            current_trip.max_battery = doc["max_battery"] | 0.0;
            current_trip.min_battery = doc["min_battery"] | 100.0;
            current_trip.max_temperature = doc["max_temperature"] | 0.0;
            current_trip.consumed_energy= doc["cunsumed_energy"] | 0.0;
            current_trip.trip_economy = doc["trip_economy"] | 0.0;

            wheelctl_data[WHEELCTL_TRIP].max_value = current_trip.trip;
            wheelctl_data[WHEELCTL_RIDETIME].value = current_trip.ride_time;
            wheelctl_data[WHEELCTL_SPEED].max_value = current_trip.max_speed;
            wheelctl_data[WHEELCTL_SPEED].min_value = current_trip.avg_speed;
            wheelctl_data[WHEELCTL_CURRENT].max_value = current_trip.max_current;
            wheelctl_data[WHEELCTL_CURRENT].min_value = current_trip.max_regen_current;
            wheelctl_data[WHEELCTL_POWER].max_value = current_trip.max_power;
            wheelctl_data[WHEELCTL_BATTPCT].max_value = current_trip.max_battery;
            wheelctl_data[WHEELCTL_BATTPCT].min_value = current_trip.min_battery;
            wheelctl_data[WHEELCTL_TEMP].max_value = current_trip.max_temperature;
            wheelctl_data[WHEELCTL_POWERCONS].value = current_trip.consumed_energy;
            wheelctl_data[WHEELCTL_ECONOMY].value = current_trip.trip_economy;
        }
        doc.clear();
    }
    file.close();
}

void wheelctl_reset_trip( void ) {
    motor_vibe(5, true);
    current_trip.ride_time = 0.0;
    current_trip.trip = 0.0;
    current_trip.max_speed = 0.0;
    current_trip.avg_speed = 0.0;
    current_trip.max_current = 0.0;
    current_trip.max_regen_current = 0.0;
    current_trip.max_power = 0.0;
    current_trip.max_battery = 0.0;
    current_trip.min_battery = 100.0;
    current_trip.max_temperature =  0.0;
    current_trip.consumed_energy= 0.0;
    current_trip.trip_economy = 0.0;

    wheelctl_data[WHEELCTL_TRIP].max_value = current_trip.trip;
    wheelctl_data[WHEELCTL_RIDETIME].value = current_trip.ride_time;
    wheelctl_data[WHEELCTL_SPEED].max_value = current_trip.max_speed;
    wheelctl_data[WHEELCTL_SPEED].min_value = current_trip.avg_speed;
    wheelctl_data[WHEELCTL_CURRENT].max_value = current_trip.max_current;
    wheelctl_data[WHEELCTL_CURRENT].min_value = current_trip.max_regen_current;
    wheelctl_data[WHEELCTL_POWER].max_value = current_trip.max_power;
    wheelctl_data[WHEELCTL_BATTPCT].max_value = current_trip.max_battery;
    wheelctl_data[WHEELCTL_BATTPCT].min_value = current_trip.min_battery;
    wheelctl_data[WHEELCTL_TEMP].max_value = current_trip.max_temperature;
    wheelctl_data[WHEELCTL_POWERCONS].value = current_trip.consumed_energy;
    wheelctl_data[WHEELCTL_ECONOMY].value = current_trip.trip_economy;
}

bool wheelctl_get_config( int config ) {
    if ( config < WHEELCTL_CONFIG_NUM ) {
        return( wheelctl_config[ config ].enable );
    }
    return false;
}

void wheelctl_set_config( int config, bool enable ) {
    if ( config < WHEELCTL_CONFIG_NUM ) {
        wheelctl_config[ config ].enable = enable;
        wheelctl_save_config();
        simpledash_tile_reload();
    }
}