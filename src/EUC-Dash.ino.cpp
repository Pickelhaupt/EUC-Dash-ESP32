# 1 "C:\\Users\\ortlundj.G06\\AppData\\Local\\Temp\\tmp2jxf6ltx"
#include <Arduino.h>
# 1 "C:/Users/ortlundj.G06/Documents/platformio/EUC-Dash-ESP32/src/EUC-Dash.ino"
# 21 "C:/Users/ortlundj.G06/Documents/platformio/EUC-Dash-ESP32/src/EUC-Dash.ino"
#include "config.h"
#include <Arduino.h>
#include "esp_bt.h"
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

TTGOClass *ttgo = TTGOClass::getWatch();
void setup();
void loop();
#line 45 "C:/Users/ortlundj.G06/Documents/platformio/EUC-Dash-ESP32/src/EUC-Dash.ino"
void setup()
{
    Serial.begin(115200);
    Serial.printf("starting t-watch V1, version: " __FIRMWARE__ " core: %d\r\n", xPortGetCoreID() );
    Serial.printf("Configure watchdog to 30s: %d\r\n", esp_task_wdt_init( 30, true ) );

    ttgo->begin();


    ttgo->lvgl_begin();

    SPIFFS.begin();
    motor_setup();


    heap_caps_malloc_extmem_enable( 1 );

    dashboard_setup();

    display_setup();

    splash_screen_stage_one();
    splash_screen_stage_update( "init serial", 10 );

    splash_screen_stage_update( "init spiff", 20 );
    if ( !SPIFFS.begin() ) {
        splash_screen_stage_update( "format spiff", 30 );
        SPIFFS.format();
        splash_screen_stage_update( "format spiff done", 40 );
        delay(500);
        bool remount_attempt = SPIFFS.begin();
        if (!remount_attempt){
            splash_screen_stage_update( "Err: SPIFF Failed", 0 );
            delay(3000);
            ESP.restart();
        }
    }

    splash_screen_stage_update( "init powermgm", 60 );
    powermgm_setup();

    splash_screen_stage_update( "init wifi", 70 );
    if ( wifictl_get_autoon() && ( pmu_is_charging() || pmu_is_vbus_plug() || ( pmu_get_battery_voltage() > 3400) ) )
        wifictl_on();


    splash_screen_stage_update( "alloc heap", 80 );
    heap_caps_malloc_extmem_enable( 16*1024 );

    splash_screen_stage_update( "init wheel data", 90 );
    wheelctl_setup();
    splash_screen_stage_update( "init gui", 100 );
    gui_setup();



    blectl_scan_setup();

    splash_screen_stage_finish();

    display_set_brightness( display_get_brightness() );

    delay(500);

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