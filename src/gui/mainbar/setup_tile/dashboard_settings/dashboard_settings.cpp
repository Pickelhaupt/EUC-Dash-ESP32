/****************************************************************************
 *   2020 Jesper Ortlund
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
#include "dashboard_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/mainbar/setup_tile/eucdash_settings/eucdash_settings.h"
#include "gui/setup.h"

#include "hardware/bma.h"
#include "hardware/motor.h"
#include "hardware/dashboard.h"

lv_obj_t *dashboard_settings_tile=NULL;
lv_style_t dashboard_settings_style;
lv_style_t dashboard_settings_heading_style;
lv_style_t dashboard_settings_data_style;
uint32_t dashboard_tile_num;

lv_obj_t *simple_onoff=NULL;
lv_obj_t *current_onoff=NULL;
lv_obj_t *bars_onoff=NULL;
lv_obj_t *distunit_onoff=NULL;
lv_obj_t *tempunit_onoff=NULL;

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(dashboard_64px);
LV_IMG_DECLARE(dashboard_32px);

static void enter_dashboard_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_dashboard_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void simple_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void current_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void bars_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void distunit_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void tempunit_onoff_event_handler(lv_obj_t * obj, lv_event_t event);

void dashboard_settings_tile_pre_setup( void ) {
    eucdash_settings_register_menu_item(&dashboard_32px, enter_dashboard_setup_event_cb, "dashboard settings");
}

uint32_t dashboard_settings_get_tile_num( void ) {
    return dashboard_tile_num;
}

void dashboard_settings_tile_setup( void ) {
    // get an app tile and copy mainstyle
    dashboard_tile_num = setup_get_submenu_tile_num();
    dashboard_settings_tile = mainbar_get_tile_obj( dashboard_tile_num );
    lv_obj_clean(dashboard_settings_tile);
    lv_style_copy( &dashboard_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &dashboard_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &dashboard_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &dashboard_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &dashboard_settings_heading_style, &dashboard_settings_style );
    lv_style_set_text_color( &dashboard_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_style_copy( &dashboard_settings_data_style, &dashboard_settings_style );
    lv_style_set_text_color( &dashboard_settings_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );
    
    lv_obj_add_style( dashboard_settings_tile, LV_OBJ_PART_MAIN, &dashboard_settings_style );

    //icon_t *dashboard_setup_icon = setup_register( "dashboard", &dashboard_64px, enter_dashboard_setup_event_cb );
    //setup_hide_indicator( dashboard_setup_icon );

    lv_obj_t *exit_btn = lv_imgbtn_create( dashboard_settings_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &dashboard_settings_style );
    lv_obj_align( exit_btn, dashboard_settings_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_dashboard_setup_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( dashboard_settings_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &dashboard_settings_heading_style  );
    lv_label_set_text( exit_label, "dashboard settings");
    lv_obj_align( exit_label, dashboard_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 15 );

    lv_obj_t *distunit_cont = lv_obj_create( dashboard_settings_tile, NULL );
    lv_obj_set_size(distunit_cont, lv_disp_get_hor_res( NULL ) , 40);
    lv_obj_add_style( distunit_cont, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_obj_align( distunit_cont, dashboard_settings_tile, LV_ALIGN_IN_TOP_RIGHT, 0, 45 );
    distunit_onoff = lv_switch_create( distunit_cont, NULL );
    lv_obj_add_protect( distunit_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( distunit_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( distunit_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( distunit_onoff, LV_ANIM_ON );
    lv_obj_align( distunit_onoff, distunit_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( distunit_onoff, distunit_onoff_event_handler );
    lv_obj_t *distunit_label = lv_label_create( distunit_cont, NULL);
    lv_obj_add_style( distunit_label, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_label_set_text( distunit_label, "imperial dist units");
    lv_obj_align( distunit_label, distunit_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *tempunit_cont = lv_obj_create( dashboard_settings_tile, NULL );
    lv_obj_set_size(tempunit_cont, lv_disp_get_hor_res( NULL ) , 40);
    lv_obj_add_style( tempunit_cont, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_obj_align( tempunit_cont, distunit_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    tempunit_onoff = lv_switch_create( tempunit_cont, NULL );
    lv_obj_add_protect( tempunit_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( tempunit_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( tempunit_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( tempunit_onoff, LV_ANIM_ON );
    lv_obj_align( tempunit_onoff, tempunit_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( tempunit_onoff, tempunit_onoff_event_handler );
    lv_obj_t *tempunit_label = lv_label_create( tempunit_cont, NULL);
    lv_obj_add_style( tempunit_label, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_label_set_text( tempunit_label, "imperial temp units");
    lv_obj_align( tempunit_label, tempunit_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *simple_cont = lv_obj_create( dashboard_settings_tile, NULL );
    lv_obj_set_size(simple_cont, lv_disp_get_hor_res( NULL ) , 40);
    lv_obj_add_style( simple_cont, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_obj_align( simple_cont, tempunit_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    simple_onoff = lv_switch_create( simple_cont, NULL );
    lv_obj_add_protect( simple_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( simple_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( simple_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( simple_onoff, LV_ANIM_ON );
    lv_obj_align( simple_onoff, simple_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( simple_onoff, simple_onoff_event_handler );
    lv_obj_t *simple_label = lv_label_create( simple_cont, NULL);
    lv_obj_add_style( simple_label, LV_OBJ_PART_MAIN, &dashboard_settings_heading_style  );
    lv_label_set_text( simple_label, "return to simpledash");
    lv_obj_align( simple_label, simple_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *bars_cont = lv_obj_create( dashboard_settings_tile, NULL );
    lv_obj_set_size(bars_cont, lv_disp_get_hor_res( NULL ) , 40);
    lv_obj_add_style( bars_cont, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_obj_align( bars_cont, simple_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    bars_onoff = lv_switch_create( bars_cont, NULL );
    lv_obj_add_protect( bars_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( bars_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( bars_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( bars_onoff, LV_ANIM_ON );
    lv_obj_align( bars_onoff, bars_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( bars_onoff, bars_onoff_event_handler );
    lv_obj_t *bars_label = lv_label_create( bars_cont, NULL);
    lv_obj_add_style( bars_label, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_label_set_text( bars_label, "display min/max bars");
    lv_obj_align( bars_label, bars_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *current_cont = lv_obj_create( dashboard_settings_tile, NULL );
    lv_obj_set_size(current_cont, lv_disp_get_hor_res( NULL ) , 40);
    lv_obj_add_style( current_cont, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_obj_align( current_cont, bars_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    current_onoff = lv_switch_create( current_cont, NULL );
    lv_obj_add_protect( current_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( current_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_obj_add_style( current_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    lv_switch_off( current_onoff, LV_ANIM_ON );
    lv_obj_align( current_onoff, current_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( current_onoff, current_onoff_event_handler );
    lv_obj_t *current_label = lv_label_create( current_cont, NULL);
    lv_obj_add_style( current_label, LV_OBJ_PART_MAIN, &dashboard_settings_style  );
    lv_label_set_text( current_label, "display current arc");
    lv_obj_align( current_label, current_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    if ( dashboard_get_config( DASHBOARD_IMPDIST ) )
        lv_switch_on( distunit_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( distunit_onoff, LV_ANIM_OFF );

    if ( dashboard_get_config( DASHBOARD_IMPTEMP ) )
        lv_switch_on( tempunit_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( tempunit_onoff, LV_ANIM_OFF );

    if ( dashboard_get_config( DASHBOARD_SIMPLE ) )
        lv_switch_on( simple_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( simple_onoff, LV_ANIM_OFF );

    if ( dashboard_get_config( DASHBOARD_BARS ) )
        lv_switch_on( bars_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( bars_onoff, LV_ANIM_OFF );

    if ( dashboard_get_config( DASHBOARD_CURRENT ) )
        lv_switch_on( current_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( current_onoff, LV_ANIM_OFF );
}

static void enter_dashboard_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       dashboard_settings_tile_setup();
                                        mainbar_jump_to_tilenumber( dashboard_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_dashboard_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( eucdash_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

static void distunit_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  dashboard_set_config( DASHBOARD_IMPDIST, lv_switch_get_state( obj ) );
    }
}

static void tempunit_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  dashboard_set_config( DASHBOARD_IMPTEMP, lv_switch_get_state( obj ) );
    }
}

static void simple_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  dashboard_set_config( DASHBOARD_SIMPLE, lv_switch_get_state( obj ) );
    }
}

static void bars_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  dashboard_set_config( DASHBOARD_BARS, lv_switch_get_state( obj ) );
    }
}

static void current_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  dashboard_set_config( DASHBOARD_CURRENT, lv_switch_get_state( obj ) );
    }
}