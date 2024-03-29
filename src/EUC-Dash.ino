/****************************************************************************
    2020 Jesper Ortlund
    derived from My-TTGO-Watch by Dirk Brosswick
 ****************************************************************************/

/*
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
#include "config.h"
#include <Arduino.h>
//#include "esp_bt.h"
#include "esp_task_wdt.h"
#include <TTGO.h>

#include "gui/gui.h"
#include "gui/splashscreen.h"

#include "hardware/display.h"
#include "hardware/powermgm.h"
#include "hardware/motor.h"
#include "hardware/wifictl.h"
#include "hardware/blectl.h"
#include "hardware/pmu.h"
#include "hardware/timesync.h"
#include "hardware/framebuffer.h"
#include "hardware/callback.h"
#include "hardware/Kingsong.h"
#include "hardware/dashboard.h"
#include "hardware/wheelctl.h"
#include "hardware/wheeldb.h"

TTGOClass *ttgo = TTGOClass::getWatch();

void setup()
{
    Serial.begin(115200);
    Serial.printf("starting t-watch V1, version: " __FIRMWARE__ " core: %d\r\n", xPortGetCoreID() );
    Serial.printf("Configure watchdog to 30s: %d\r\n", esp_task_wdt_init( 30, true ) );
    
    Serial.printf("init t-watch\r\n");
    ttgo->begin();

    // force to store all new heap allocations in psram to get more internal ram
    //heap_caps_malloc_extmem_enable( 1 );
    //heap_caps_malloc_extmem_enable( 16*1024 );
    
    Serial.printf("init LVGL\r\n");
    ttgo->lvgl_begin();
    Serial.printf("init SPIFFS\r\n");
    SPIFFS.begin();

    Serial.printf("starting display\r\n");
    display_setup();
    splash_screen_stage_one();
    splash_screen_stage_update( "booting....", 0 );

    splash_screen_stage_update( "init haptic", 10 );
    motor_setup();

    /*
    SPIFFS.end();
    log_i("SPIFFS unmounted!");
    delay(100);
    SPIFFS.format();
    log_i("SPIFFS format complete!");
    */

    splash_screen_stage_update( "init dashboard", 20 );
    dashboard_setup();

    
    splash_screen_stage_update( "init spiffs", 30 );
    if ( !SPIFFS.begin() ) {
        splash_screen_stage_update( "format spiffs", 30 );
        SPIFFS.format();
        splash_screen_stage_update( "format spiffs done", 40 );
        delay(500);
        bool remount_attempt = SPIFFS.begin();
        if (!remount_attempt){
            splash_screen_stage_update( "Err: SPIFFS Failed", 40 );
            delay(3000);
            ESP.restart();
        }
    }

    splash_screen_stage_update( "init powermgm", 50 );
    powermgm_setup();
    
    splash_screen_stage_update( "init wifi", 60 );
    if ( wifictl_get_autoon() && ( pmu_is_charging() || pmu_is_vbus_plug() || ( pmu_get_battery_voltage() > 3400) ) )
        wifictl_on();

    // enable to store data in normal heap
    splash_screen_stage_update( "alloc heap", 75 );
    //heap_caps_malloc_extmem_enable( 16*1024 );
    wheeldb_setup();
    splash_screen_stage_update( "init wheel data", 90 );
    wheelctl_setup();
    splash_screen_stage_update( "init gui", 100 );
    splash_screen_stage_update( "init gui", 100 );
    gui_setup();

    //blectl_setup();
    blectl_scan_setup();

    splash_screen_stage_finish();

    display_set_brightness( display_get_brightness() );

    delay(300);

    Serial.printf("Total heap: %d\r\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\r\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\r\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\r\n", ESP.getFreePsram());

    disableCore0WDT();
    callback_print();
}

void loop() {
    powermgm_loop();
}
