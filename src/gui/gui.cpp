/****************************************************************************
 *   Modified 2021 Jesper Ortlund
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
#include "setup.h"

#include "mainbar/mainbar.h"

#include "mainbar/main_tile/main_tile.h"
#include "mainbar/dashboard_tile/dashboard_tile.h"
#include "mainbar/setup_tile/setup_tile.h"
#include "mainbar/tripinfo_tile/tripinfo_tile.h"
#include "mainbar/wheelinfo_tile/wheelinfo_tile.h"

#include "mainbar/setup_tile/watch_settings/watch_settings.h"
#include "mainbar/setup_tile/eucdash_settings/eucdash_settings.h"

#include "hardware/powermgm.h"
#include "hardware/display.h"

bool gui_powermgm_event_cb( EventBits_t event, void *arg );
bool gui_powermgm_loop_event_cb( EventBits_t event, void *arg );

void gui_setup( void )
{
    /* Setup the tile view*/
    log_i("setting up mainbar");
    mainbar_setup();

    /* add the five mainbar screens */
    log_i("setting up main tile");
    main_tile_setup();
    log_i("setting up full dash");
    dashboard_tile_setup();
    log_i("setting up setup tile");
    setup_tile_setup();
    log_i("setting up trip info");
    tripinfo_tile_setup();
    log_i("setting up wheel info");
    wheelinfo_tile_setup();
    
    /* add setup screens */
    log_i("watch settings");
    watch_settings_tile_setup();
    log_i("eucdash settings");
    eucdash_settings_tile_setup();
    log_i("adding setup screens");
    setup_add_submenu_tile();
    setup_add_submenu2_tile();
    
    lv_disp_trig_activity( NULL );
     log_i("setting up keyboard");
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
