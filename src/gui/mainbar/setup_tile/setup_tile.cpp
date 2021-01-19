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
#include "setup_tile.h"

#include "gui/mainbar/mainbar.h"
#include "gui/icon.h"
#include "hardware/motor.h"
#include "hardware/blectl.h"
#include "hardware/wheelctl.h"
#include "hardware/display.h"

icon_t setup_entry[ MAX_SETUP_ICON ];


lv_obj_t *setup_cont[ MAX_SETUP_TILES ];
uint32_t setup_tile_num[ MAX_SETUP_TILES ];
lv_style_t setup_style;
lv_style_t setup_heading_style;
lv_style_t setup_data_style;
lv_style_t setup_no_data_style;

lv_obj_t *brightness_slider = NULL;
lv_obj_t *wheel_connect_onoff=NULL;
lv_obj_t *connect_data;

LV_FONT_DECLARE(DIN1451_m_cond_28);
LV_FONT_DECLARE(DIN1451_m_cond_24);
LV_IMG_DECLARE(brightness_32px);

static void brightness_setup_event_cb( lv_obj_t * obj, lv_event_t event );
bool setup_displayctl_brightness_event_cb( EventBits_t event, void *arg );
static void toggle_wheel_connect_onoff_event_handler(lv_obj_t * obj, lv_event_t event);

void setup_tile_setup( void ) {

    
    for ( int tiles = 0 ; tiles < MAX_SETUP_TILES ; tiles++ ) {
        setup_tile_num[ tiles ] = mainbar_add_tile( 0 + tiles , 1, "setup tile" );
        setup_cont[ tiles ] = mainbar_get_tile_obj( setup_tile_num[ tiles ] );
    }

    lv_style_copy( &setup_style, mainbar_get_style() );
    lv_style_set_text_font( &setup_style, LV_STATE_DEFAULT, &DIN1451_m_cond_24);

    lv_style_copy( &setup_heading_style, &setup_style );
    lv_style_set_text_color( &setup_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_style_copy( &setup_data_style, &setup_style );
    lv_style_set_text_color( &setup_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );
    
    lv_style_copy( &setup_no_data_style, &setup_style );
    lv_style_set_text_color( &setup_no_data_style, LV_OBJ_PART_MAIN, LV_COLOR_RED );

    lv_obj_t *setup_label = lv_label_create( setup_cont[ 0 ], NULL );
    lv_obj_add_style(setup_label,  LV_OBJ_PART_MAIN, &setup_heading_style  );
    lv_label_set_text(setup_label, "settings");
    lv_obj_align(setup_label, setup_cont[ 0 ], LV_ALIGN_IN_TOP_MID, 0, 5 );

    lv_obj_t *brightness_cont = lv_obj_create( setup_cont[ 0 ], NULL );
    lv_obj_set_size( brightness_cont, lv_disp_get_hor_res( NULL ), 35 );
    lv_obj_add_style( brightness_cont, LV_OBJ_PART_MAIN, &setup_style  );
    lv_obj_align( brightness_cont, setup_cont[ 0 ], LV_ALIGN_IN_TOP_MID, 0, 55 );
    brightness_slider = lv_slider_create( brightness_cont, NULL );
    lv_obj_add_protect( brightness_slider, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( brightness_slider, LV_SLIDER_PART_INDIC, mainbar_get_slider_style() );
    lv_obj_add_style( brightness_slider, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_slider_set_range( brightness_slider, DISPLAY_MIN_BRIGHTNESS, DISPLAY_MAX_BRIGHTNESS );
    lv_obj_set_size( brightness_slider, lv_disp_get_hor_res( NULL ) - 60 , 15 );
    lv_obj_align( brightness_slider, brightness_cont, LV_ALIGN_IN_RIGHT_MID, -10, 0 );
    lv_obj_set_event_cb( brightness_slider, brightness_setup_event_cb );
    lv_obj_t *brightness_icon = lv_img_create( brightness_cont, NULL );
    lv_img_set_src( brightness_icon, &brightness_32px );
    lv_obj_align( brightness_icon, brightness_cont, LV_ALIGN_IN_LEFT_MID, 10, 0 );
    lv_slider_set_value( brightness_slider, display_get_brightness(), LV_ANIM_OFF );
    lv_tileview_add_element( setup_cont[0], brightness_cont );
    display_register_cb( DISPLAYCTL_BRIGHTNESS, setup_displayctl_brightness_event_cb, "display settings" );

    lv_obj_t *brightness_label = lv_label_create( setup_cont[ 0 ], NULL );
    lv_obj_add_style(brightness_label,  LV_OBJ_PART_MAIN, &setup_style  );
    lv_label_set_text(brightness_label, "display brightness");
    lv_obj_align(brightness_label, brightness_cont, LV_ALIGN_OUT_TOP_MID, 0, 3);

    lv_obj_t *wheel_connect_cont = lv_obj_create( setup_cont[ 0 ], NULL );
    lv_page_glue_obj(wheel_connect_cont, true);
    lv_obj_set_size(wheel_connect_cont, lv_disp_get_hor_res( NULL ), 35);
    lv_obj_add_style( wheel_connect_cont, LV_OBJ_PART_MAIN, &setup_style  );
    lv_obj_align( wheel_connect_cont, brightness_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, -5 );
    wheel_connect_onoff = lv_switch_create( wheel_connect_cont, NULL );
    lv_obj_add_protect( wheel_connect_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( wheel_connect_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( wheel_connect_onoff, LV_SWITCH_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( wheel_connect_onoff, LV_ANIM_ON );
    lv_obj_align( wheel_connect_onoff, wheel_connect_cont, LV_ALIGN_IN_RIGHT_MID, -10, 0 );
    lv_obj_set_event_cb( wheel_connect_onoff, toggle_wheel_connect_onoff_event_handler );
    lv_obj_t *wheel_connect_label = lv_label_create( wheel_connect_cont, NULL);
    lv_obj_add_style( wheel_connect_label, LV_OBJ_PART_MAIN, &setup_style  );
    lv_label_set_text( wheel_connect_label, "connect to wheel");
    lv_obj_align( wheel_connect_label, wheel_connect_cont, LV_ALIGN_IN_LEFT_MID, 10, 0 );
    if ( blectl_get_autoconnect() ) lv_switch_on( wheel_connect_onoff, LV_ANIM_OFF );
    else lv_switch_off( wheel_connect_onoff, LV_ANIM_OFF );

    lv_obj_t *connect_label = lv_label_create( setup_cont[ 0 ], NULL);
    lv_obj_add_style( connect_label, LV_OBJ_PART_MAIN, &setup_style );
    lv_label_set_text( connect_label, "connected to:");
    lv_obj_align( connect_label, wheel_connect_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 0 );
    connect_data = lv_label_create( setup_cont[ 0 ], NULL);
    lv_obj_add_style( connect_data, LV_OBJ_PART_MAIN, &setup_data_style  );
    lv_label_set_text( connect_data, "disconnected");
    lv_obj_align( connect_data, wheel_connect_cont, LV_ALIGN_OUT_BOTTOM_RIGHT, -10, 0 );

    for ( int setup = 0 ; setup < MAX_SETUP_ICON ; setup++ ) {
        // set x, y and mark it as inactive
        setup_entry[ setup ].x = SETUP_FIRST_X_POS + ( ( setup % MAX_SETUP_ICON_HORZ ) * ( SETUP_ICON_X_SIZE + SETUP_ICON_X_CLEARENCE ) );
        setup_entry[ setup ].y = SETUP_FIRST_Y_POS + ( ( ( setup % ( MAX_SETUP_ICON_VERT * MAX_SETUP_ICON_HORZ ) ) / MAX_SETUP_ICON_HORZ ) * ( SETUP_ICON_Y_SIZE + SETUP_ICON_Y_CLEARENCE ) );
        setup_entry[ setup ].active = false;
        // create app icon container
        setup_entry[ setup ].icon_cont = mainbar_obj_create( setup_cont[ setup / ( MAX_SETUP_ICON_HORZ * MAX_SETUP_ICON_VERT ) ] );
        lv_obj_reset_style_list( setup_entry[ setup ].icon_cont, LV_OBJ_PART_MAIN );
        lv_obj_add_style( setup_entry[ setup ].icon_cont, LV_OBJ_PART_MAIN, &setup_style );
        lv_obj_set_size( setup_entry[ setup ].icon_cont, SETUP_ICON_X_SIZE, SETUP_ICON_Y_SIZE );
        //lv_obj_align( setup_entry[ setup ].icon_cont , setup_cont[ setup / ( MAX_SETUP_ICON_HORZ * MAX_SETUP_ICON_VERT ) ], LV_ALIGN_CENTER, setup_entry[ setup ].x, setup_entry[ setup ].y );
        lv_obj_align( setup_entry[ setup ].icon_cont , setup_cont[ setup / ( MAX_SETUP_ICON_HORZ * MAX_SETUP_ICON_VERT ) ], LV_ALIGN_IN_TOP_LEFT, setup_entry[ setup ].x, setup_entry[ setup ].y );
        // create app label
        
        setup_entry[ setup ].label = lv_label_create( setup_cont[ setup / ( MAX_SETUP_ICON_HORZ * MAX_SETUP_ICON_VERT ) ], NULL );
        mainbar_add_slide_element(setup_entry[ setup ].label);
        lv_obj_reset_style_list( setup_entry[ setup ].label, LV_OBJ_PART_MAIN );
        lv_obj_add_style( setup_entry[ setup ].label, LV_OBJ_PART_MAIN, &setup_heading_style );
        lv_obj_set_size( setup_entry[ setup ].label, SETUP_LABEL_X_SIZE, SETUP_LABEL_Y_SIZE );
        lv_obj_align( setup_entry[ setup ].label , setup_entry[ setup ].icon_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
        
        lv_obj_set_hidden( setup_entry[ setup ].icon_cont, true );
        lv_obj_set_hidden( setup_entry[ setup ].label, true );

        log_d("icon screen/x/y: %d/%d/%d", setup / ( MAX_SETUP_ICON_HORZ * MAX_SETUP_ICON_VERT ), setup_entry[ setup ].x, setup_entry[ setup ].y );
    }
}

bool setup_displayctl_brightness_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case DISPLAYCTL_BRIGHTNESS:
            lv_slider_set_value( brightness_slider, display_get_brightness() , LV_ANIM_OFF );
            break;
    }
    return( true );
}

static void brightness_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED ):     display_set_brightness( lv_slider_get_value( obj ) );
                                            break;
    }
}

static void toggle_wheel_connect_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  blectl_set_autoconnect(lv_switch_get_state( obj ));
                                        if(lv_switch_get_state( obj )) blectl_scan_once(2, false);
                                        break;
    }
}

lv_obj_t *setup_tile_register_setup( void ) {
    for( int setup = 0 ; setup < MAX_SETUP_ICON ; setup++ ) {
        if ( setup_entry[ setup ].active == false ) {
            setup_entry[ setup ].active = true;
            lv_obj_set_hidden( setup_entry[ setup ].icon_cont, false );
            return( setup_entry[ setup ].icon_cont );
        }
    }
    log_e("no space for an setup icon");
    return( NULL );
}

icon_t *setup_tile_get_free_setup_icon( void ) {
    for( int setup = 0 ; setup < MAX_SETUP_ICON ; setup++ ) {
        if ( setup_entry[ setup ].active == false ) {
            return( &setup_entry[ setup ] );
        }
    }
    log_e("no space for an setup icon");
    return( NULL );
}

uint32_t setup_get_tile_num( void ) {
    return( setup_tile_num[ 0 ] );
}

void setup_tile_connect_update( String connect_string){
    lv_label_set_text( connect_data, connect_string.c_str());
    lv_obj_realign(connect_data);
}
