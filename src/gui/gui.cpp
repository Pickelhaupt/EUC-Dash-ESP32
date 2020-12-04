/****************************************************************************
 *   Modified 2020 Jesper Ortlund
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
#include <stdio.h>
#include <TTGO.h>

#include "gui.h"
#include "keyboard.h"

#include "mainbar/mainbar.h"

#include "mainbar/main_tile/main_tile.h"
#include "mainbar/fulldash_tile/fulldash_tile.h"
#include "mainbar/simpledash_tile/simpledash_tile.h"
#include "mainbar/setup_tile/setup_tile.h"
#include "mainbar/tripinfo_tile/tripinfo_tile.h"
#include "mainbar/wheelinfo_tile/wheelinfo_tile.h"

#include "mainbar/setup_tile/battery_settings/battery_settings.h"
#include "mainbar/setup_tile/display_settings/display_settings.h"
#include "mainbar/setup_tile/wheel_settings/wheel_settings.h"
#include "mainbar/setup_tile/time_settings/time_settings.h"
#include "mainbar/setup_tile/update/update.h"
#include "mainbar/setup_tile/wlan_settings/wlan_settings.h"
#include "mainbar/setup_tile/bluetooth_settings/bluetooth_settings.h"
#include "mainbar/setup_tile/dashboard_settings/dashboard_settings.h"
#include "mainbar/setup_tile/utilities/utilities.h"

#include "hardware/powermgm.h"
#include "hardware/display.h"



bool gui_powermgm_event_cb( EventBits_t event, void *arg );
bool gui_powermgm_loop_event_cb( EventBits_t event, void *arg );

void gui_setup( void )
{
    /* Setup the tile view*/
    mainbar_setup();

    /* add the six mainbar screens */
    main_tile_setup();
    fulldash_tile_setup();
    simpledash_tile_setup();
    setup_tile_setup();
    tripinfo_tile_setup();
    wheelinfo_tile_setup();

    /* add setup screens */
    battery_settings_tile_setup();
    display_settings_tile_setup();
    wheel_settings_tile_setup();
    wlan_settings_tile_setup();
    bluetooth_settings_tile_setup();
    time_settings_tile_setup();
    update_tile_setup();
    utilities_tile_setup();
    dashboard_settings_tile_setup();

    lv_disp_trig_activity( NULL );

    keyboard_setup();

    powermgm_register_cb( POWERMGM_STANDBY | POWERMGM_WAKEUP | POWERMGM_SILENCE_WAKEUP, gui_powermgm_event_cb, "gui" );
    powermgm_register_loop_cb( POWERMGM_WAKEUP | POWERMGM_SILENCE_WAKEUP, gui_powermgm_loop_event_cb, "gui loop" );
}

bool gui_powermgm_event_cb( EventBits_t event, void *arg ) {
    TTGOClass *ttgo = TTGOClass::getWatch();

    switch ( event ) {
        case POWERMGM_STANDBY:          log_i("go standby");
                                        if ( !display_get_block_return_maintile() ) {
                                            mainbar_jump_to_maintile( LV_ANIM_OFF );
                                        }                               
                                        ttgo->stopLvglTick();
                                        break;
        case POWERMGM_WAKEUP:           log_i("go wakeup");
                                        ttgo->startLvglTick();
                                        lv_disp_trig_activity( NULL );
                                        break;
        case POWERMGM_SILENCE_WAKEUP:   log_i("go silence wakeup");
                                        ttgo->startLvglTick();
                                        lv_disp_trig_activity( NULL );
                                        break;
    }
    return( true );
}

bool gui_powermgm_loop_event_cb( EventBits_t event, void *arg ) {
    switch ( event ) {
        case POWERMGM_WAKEUP:           if ( lv_disp_get_inactive_time( NULL ) < display_get_timeout() * 1000 || display_get_timeout() == DISPLAY_MAX_TIMEOUT ) {
                                            lv_task_handler();
                                            if(LV_EVENT_VALUE_CHANGED) {
                                               // mainbar_tilevent_action();
                                            }
                                        }
                                        else {
                                            powermgm_set_event( POWERMGM_STANDBY_REQUEST );
                                        }
                                        break;
        case POWERMGM_SILENCE_WAKEUP:   if ( lv_disp_get_inactive_time( NULL ) < display_get_timeout() * 1000 ) {
                                            lv_task_handler();
                                        }
                                        else {
                                            powermgm_set_event( POWERMGM_STANDBY_REQUEST );
                                        }
                                        break;
    }
    return( true );
}
