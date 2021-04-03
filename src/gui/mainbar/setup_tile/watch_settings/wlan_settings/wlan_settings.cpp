/****************************************************************************
 *   Modified 2020 Jesper Ortlund  
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
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
#include "wlan_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/keyboard.h"
#include "gui/setup.h"

#include "hardware/wifictl.h"
#include "hardware/motor.h"
#include "hardware/blectl.h"
#include "hardware/json_psram_allocator.h"
#include "gui/mainbar/setup_tile/watch_settings/watch_settings.h"

#include <WiFi.h>

LV_IMG_DECLARE(exit_32px);

lv_obj_t *wifi_setup_tile=NULL;
lv_style_t wifi_setup_style;
uint32_t wifi_setup_tile_num;

static void enter_wifi_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wifi_setup_event_cb( lv_obj_t * obj, lv_event_t event );

bool wifi_setup_bluetooth_message_event_cb( EventBits_t event, void *arg );
static void wifi_setup_bluetooth_message_msg_pharse( const char* msg );

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(wifi_32px);
LV_IMG_DECLARE(setup_32px);

lv_obj_t *wifi_autoon_onoff = NULL;
lv_obj_t *wifi_enabled_on_standby_onoff = NULL;
static void wps_start_event_handler( lv_obj_t * obj, lv_event_t event );
static void wifi_autoon_onoff_event_handler( lv_obj_t * obj, lv_event_t event );
static void wifi_enabled_on_standby_onoff_event_handler( lv_obj_t * obj, lv_event_t event );
bool wifi_setup_autoon_event_cb( EventBits_t event, void *arg );

void wlan_setup_tile_pre_setup( void ){
    watch_settings_register_menu_item(&wifi_32px, enter_wifi_setup_event_cb, "wifi setup");
}

void wlan_setup_tile_setup() {
    // get an app tile and copy mainstyle
    wifi_setup_tile_num = setup_get_submenu_tile_num();
    wifi_setup_tile = mainbar_get_tile_obj( wifi_setup_tile_num );
    lv_obj_clean(wifi_setup_tile);
    lv_style_copy( &wifi_setup_style, mainbar_get_style() );
    lv_style_set_bg_color( &wifi_setup_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &wifi_setup_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wifi_setup_style, LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( wifi_setup_tile, LV_OBJ_PART_MAIN, &wifi_setup_style );

    lv_obj_t *exit_btn = lv_imgbtn_create( wifi_setup_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &wifi_setup_style );
    lv_obj_align( exit_btn, wifi_setup_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_wifi_setup_event_cb );

    lv_obj_t *exit_label = lv_label_create( wifi_setup_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &wifi_setup_style );
    lv_label_set_text( exit_label, "wlan settings");
    lv_obj_align( exit_label, exit_btn, LV_ALIGN_OUT_RIGHT_MID, 5, 0 );

    lv_obj_t *wifi_autoon_onoff_cont = lv_obj_create( wifi_setup_tile, NULL );
    lv_obj_set_size(wifi_autoon_onoff_cont, lv_disp_get_hor_res( NULL ) , 32);
    lv_obj_add_style( wifi_autoon_onoff_cont, LV_OBJ_PART_MAIN, &wifi_setup_style  );
    lv_obj_align( wifi_autoon_onoff_cont, wifi_setup_tile, LV_ALIGN_IN_TOP_RIGHT, 0, 75 );
    wifi_autoon_onoff = lv_switch_create( wifi_setup_tile, NULL );
    lv_obj_add_protect( wifi_autoon_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( wifi_autoon_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( wifi_autoon_onoff, LV_ANIM_ON );
    lv_obj_align( wifi_autoon_onoff, wifi_autoon_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( wifi_autoon_onoff, wifi_autoon_onoff_event_handler );
    lv_obj_t *wifi_autoon_label = lv_label_create( wifi_autoon_onoff_cont, NULL);
    lv_obj_add_style( wifi_autoon_label, LV_OBJ_PART_MAIN, &wifi_setup_style  );
    lv_label_set_text( wifi_autoon_label, "enable on wakeup");
    lv_obj_align( wifi_autoon_label, wifi_autoon_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

     lv_obj_t *wifi_enabled_on_standby_onoff_cont = lv_obj_create( wifi_setup_tile, NULL );
    lv_obj_set_size(wifi_enabled_on_standby_onoff_cont, lv_disp_get_hor_res( NULL ) , 32);
    lv_obj_add_style( wifi_enabled_on_standby_onoff_cont, LV_OBJ_PART_MAIN, &wifi_setup_style  );
    lv_obj_align( wifi_enabled_on_standby_onoff_cont, wifi_setup_tile, LV_ALIGN_IN_TOP_RIGHT, 0, 115 );
    wifi_enabled_on_standby_onoff = lv_switch_create( wifi_enabled_on_standby_onoff_cont, NULL );
    lv_obj_add_protect( wifi_enabled_on_standby_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( wifi_enabled_on_standby_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( wifi_enabled_on_standby_onoff, LV_ANIM_ON );
    lv_obj_align( wifi_enabled_on_standby_onoff, wifi_enabled_on_standby_onoff_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( wifi_enabled_on_standby_onoff, wifi_enabled_on_standby_onoff_event_handler );
    lv_obj_t *wifi_enabled_on_standby_label = lv_label_create( wifi_enabled_on_standby_onoff_cont, NULL);
    lv_obj_add_style( wifi_enabled_on_standby_label, LV_OBJ_PART_MAIN, &wifi_setup_style  );
    lv_label_set_text( wifi_enabled_on_standby_label, "enable on standby");
    lv_obj_align( wifi_enabled_on_standby_label, wifi_enabled_on_standby_onoff_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );   

    lv_obj_t *wps_btn = lv_btn_create( wifi_setup_tile, NULL);
    lv_obj_set_event_cb( wps_btn, wps_start_event_handler );
    lv_obj_align( wps_btn, wifi_enabled_on_standby_onoff_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_t *wps_btn_label = lv_label_create( wps_btn, NULL );
    lv_label_set_text( wps_btn_label, "start WPS");

    if ( wifictl_get_autoon() ) {
        lv_switch_on( wifi_autoon_onoff, LV_ANIM_OFF);
    }
    else {
        lv_switch_off( wifi_autoon_onoff, LV_ANIM_OFF);
    }

    if ( wifictl_get_enable_on_standby() ) {
        lv_switch_on( wifi_enabled_on_standby_onoff, LV_ANIM_OFF);
    }
    else {
        lv_switch_off( wifi_enabled_on_standby_onoff, LV_ANIM_OFF);
    }

    blectl_register_cb( BLECTL_MSG, wifi_setup_bluetooth_message_event_cb, "wifi settings" );
    wifictl_register_cb( WIFICTL_AUTOON, wifi_setup_autoon_event_cb, "wifi setup");
}

static void wps_start_event_handler( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       wifictl_start_wps();
                                        break;
    }
}

static void enter_wifi_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       wlan_setup_tile_setup();
                                        mainbar_jump_to_tilenumber( wifi_setup_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_wifi_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( watch_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

static void wifi_autoon_onoff_event_handler( lv_obj_t * obj, lv_event_t event ) {
    switch (event) {
        case (LV_EVENT_VALUE_CHANGED):  wifictl_set_autoon( lv_switch_get_state( obj ) );
                                        break;
    }
}


static void wifi_enabled_on_standby_onoff_event_handler( lv_obj_t * obj, lv_event_t event ) {
    switch (event) {
        case (LV_EVENT_VALUE_CHANGED):  wifictl_set_enable_on_standby( lv_switch_get_state( obj ) );
                                        break;
    }
}

bool wifi_setup_autoon_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case WIFICTL_AUTOON:
            if ( *(bool*)arg ) {
                lv_switch_on( wifi_autoon_onoff, LV_ANIM_OFF);
            }
            else {
                lv_switch_off( wifi_autoon_onoff, LV_ANIM_OFF);
            }
            break;
    }
    return( true );
}

bool wifi_setup_bluetooth_message_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case BLECTL_MSG:            wifi_setup_bluetooth_message_msg_pharse( (const char*)arg );
                                    break;
    }
    return( true );
}

void wifi_setup_bluetooth_message_msg_pharse( const char* msg ) {

    SpiRamJsonDocument doc( strlen( msg ) * 4 );

    DeserializationError error = deserializeJson( doc, msg );
    if ( error ) {
        log_e("bluetooth message deserializeJson() failed: %s", error.c_str() );
    }
    else {
        if( !strcmp( doc["t"], "conf" ) ) {
             if ( !strcmp( doc["app"], "settings" ) ) {
                if ( !strcmp( doc["settings"], "wlan" ) ) {
                    motor_vibe(100);
                    wifictl_insert_network(  doc["ssid"] |"" , doc["key"] |"" );
                }
             }

        }
    }        
    doc.clear();
}
