/****************************************************************************
 *   2021 Jesper Ortlund
 *   Derived from My-TTGO-Watch -- 2020 Dirk Brosswick
 ****************************************************************************
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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
 */

#include "wheelctlconfig.h"
#include "hardware/alloc.h"

wheelctlconfig_t::wheelctlconfig_t() : BaseJsonConfig(WHEELCTLCONFIG_JSON_FILE){
}

bool wheelctlconfig_t::onSave(JsonDocument& doc) {
        doc["lights_mode"] = lights_mode;
        doc["flash_mode"] = flash_mode;
        doc["save_light_state"] = save_light_state;
        doc["cycle_beamtype"] = cycle_beamtype;
        doc["haptic"] = haptic;
        doc["horn_while_riding"] = horn_while_riding;
        return true;
}

bool wheelctlconfig_t::onLoad(JsonDocument& doc) {
        lights_mode = doc["lights_mode"] | 0x00;
        flash_mode = doc["flash_mode"] | false;
        save_light_state = doc["save_light_state"] | false;
        cycle_beamtype = doc["cycle_beamtype"] | false;
        haptic = doc["haptic"] | false;
        horn_while_riding = doc["horn_while_riding"] | false;
        return true;
}

bool wheelctlconfig_t::onDefault( void ) {
    return true;
}

tripdata_t::tripdata_t() : BaseJsonConfig(CURRENT_TRIP_JSON_FILE) {
}

bool tripdata_t::onSave(JsonDocument& doc) {
    doc["timestamp"] = timestamp;
    doc["ride_time"] = ride_time;
    doc["trip"] = trip;
    doc["max_speed"] = max_speed;
    doc["avg_speed"] = avg_speed;
    doc["max_current"] = max_current;
    doc["max_regen_current"] = max_regen_current;
    doc["max_power"] = max_power;
    doc["max_battery"] = max_battery;
    doc["min_battery"] = min_battery;
    doc["max_temperature"] =  max_temperature;
    doc["consumed_energy"] = consumed_energy;
    doc["trip_economy"] = trip_economy;

    return true;
}

bool tripdata_t::onLoad(JsonDocument& doc) {
    timestamp = doc["timestamp"] | 0;
    ride_time = doc["ride_time"] | 0.0;
    trip = doc["trip"] | 0.0;
    max_speed = doc["max_speed"] | 0.0;
    avg_speed = doc["avg_speed"] | 0.0;
    max_current = doc["max_current"] | 0.0;
    max_regen_current = doc["max_regen_current"] | 0.0;
    max_power = doc["max_power"] | 0.0;
    max_battery = doc["max_battery"] | 0.0;
    min_battery = doc["min_battery"] | 100.0;
    max_temperature = doc["max_temperature"] | 0.0;
    consumed_energy= doc["consumed_energy"] | 0.0;
    trip_economy = doc["trip_economy"] | 0.0;

    return true;
}

bool tripdata_t::onDefault( void ) {
    return true;
}