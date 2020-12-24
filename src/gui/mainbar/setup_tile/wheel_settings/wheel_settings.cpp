/****************************************************************************
 *   Copyright  2020  Jesper Ortlund
 *   based on work by Dirk Brosswick 2020
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
#include "wheel_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/mainbar/setup_tile/eucdash_settings/eucdash_settings.h"
#include "gui/setup.h"

#include "hardware/bma.h"
#include "hardware/blectl.h"
#include "hardware/motor.h"
#include "hardware/wheelctl.h"

lv_obj_t *wheel_settings_tile=NULL;
lv_style_t wheel_settings_style;
lv_style_t wheel_page_style;
lv_style_t wheel_page_edge_style;
lv_style_t wheel_settings_heading_style;
uint32_t wheel_tile_num;

lv_obj_t *disable_startuplights_onoff=NULL;
lv_obj_t *toggle_leds_onoff=NULL;
lv_obj_t *autoconnect_onoff=NULL;
lv_obj_t *horn_press_onoff=NULL;
lv_obj_t *three_onoff=NULL;
lv_obj_t *four_onoff=NULL;
lv_obj_t *five_onoff=NULL;
lv_obj_t *six_onoff=NULL;
lv_obj_t *seven_onoff=NULL;
lv_obj_t *eight_onoff=NULL;

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(wheel_64px);
LV_IMG_DECLARE(wheel_32px);

static void enter_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void disable_startuplights_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_leds_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_horn_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_autoconnect_onoff_event_handler(lv_obj_t * obj, lv_event_t event);

void wheel_settings_tile_pre_setup( void ) {
    eucdash_settings_register_menu_item(&wheel_32px, enter_wheel_setup_event_cb, "wheel settings");
}

uint32_t wheel_settings_get_tile_num( void ) {
    return wheel_tile_num;
}

void wheel_settings_tile_setup() {
    // get an app tile and copy mainstyle
    wheel_tile_num = setup_get_submenu_tile_num();
    wheel_settings_tile = mainbar_get_tile_obj( wheel_tile_num );
    lv_obj_clean(wheel_settings_tile);
    log_i("wheel tile num: %d", wheel_tile_num);
    lv_style_copy( &wheel_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &wheel_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &wheel_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wheel_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &wheel_settings_heading_style, &wheel_settings_style );
    lv_style_set_text_color( &wheel_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_obj_add_style( wheel_settings_tile, LV_OBJ_PART_MAIN, &wheel_settings_style );

    //icon_t *wheel_setup_icon = setup_register( "wheel", &wheel_64px, enter_wheel_setup_event_cb );
    //setup_hide_indicator( wheel_setup_icon );

    lv_obj_t *exit_btn = lv_imgbtn_create( wheel_settings_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &wheel_settings_style );
    lv_obj_align( exit_btn, wheel_settings_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_wheel_setup_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( wheel_settings_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &wheel_settings_heading_style  );
    lv_label_set_text( exit_label, "wheel settings");
    lv_obj_align( exit_label, wheel_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 15 );

    lv_style_copy( &wheel_page_style, &wheel_settings_style );
    lv_style_set_pad_all(&wheel_page_style, LV_STATE_DEFAULT, 0);
    lv_style_copy( &wheel_page_edge_style, &wheel_page_style );
    lv_style_set_bg_color(&wheel_page_edge_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&wheel_page_edge_style, LV_STATE_DEFAULT, 30);
    lv_obj_t *wheel_page = lv_page_create( wheel_settings_tile, NULL);
    lv_obj_set_size(wheel_page, lv_disp_get_hor_res( NULL ), 195);
    //lv_page_set_scrl_width(wheel_page, 215);
    lv_page_set_edge_flash(wheel_page, true);
    lv_obj_add_style(wheel_page, LV_OBJ_PART_MAIN, &wheel_page_style );
    lv_obj_add_style(wheel_page, LV_PAGE_PART_EDGE_FLASH, &wheel_page_edge_style );
    lv_obj_align( wheel_page, wheel_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 45 );

    lv_obj_t *disable_startuplights_cont = lv_obj_create( wheel_page, NULL );
    lv_page_glue_obj(disable_startuplights_cont, true);
    lv_obj_set_size(disable_startuplights_cont, lv_disp_get_hor_res( NULL ) - 10, 60);
    lv_obj_add_style( disable_startuplights_cont, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_obj_align( disable_startuplights_cont, wheel_page, LV_ALIGN_IN_TOP_LEFT, 0, 0 );
    disable_startuplights_onoff = lv_switch_create( disable_startuplights_cont, NULL );
    lv_obj_add_protect( disable_startuplights_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( disable_startuplights_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( disable_startuplights_onoff, LV_ANIM_ON );
    lv_obj_align( disable_startuplights_onoff, disable_startuplights_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( disable_startuplights_onoff, disable_startuplights_onoff_event_handler );
    lv_obj_t *disable_startuplights_label = lv_label_create( disable_startuplights_cont, NULL);
    lv_obj_add_style( disable_startuplights_label, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_label_set_text( disable_startuplights_label, "Disable lights\non connect");
    lv_obj_align( disable_startuplights_label, disable_startuplights_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *toggle_leds_cont = lv_obj_create( wheel_page, NULL );
    lv_page_glue_obj(toggle_leds_cont, true);
    lv_obj_set_size(toggle_leds_cont, lv_disp_get_hor_res( NULL ) - 10, 60);
    lv_obj_add_style( toggle_leds_cont, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_obj_align( toggle_leds_cont, disable_startuplights_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    toggle_leds_onoff = lv_switch_create( toggle_leds_cont, NULL );
    lv_obj_add_protect( toggle_leds_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( toggle_leds_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( toggle_leds_onoff, LV_ANIM_ON );
    lv_obj_align( toggle_leds_onoff, toggle_leds_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( toggle_leds_onoff, toggle_leds_onoff_event_handler );
    lv_obj_t *toggle_leds_label = lv_label_create( toggle_leds_cont, NULL);
    lv_obj_add_style( toggle_leds_label, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_label_set_text( toggle_leds_label, "toggle leds when\ntoggling lights");
    lv_obj_align( toggle_leds_label, toggle_leds_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *autoconnect_cont = lv_obj_create( wheel_page, NULL );
    lv_page_glue_obj(autoconnect_cont, true);
    lv_obj_set_size(autoconnect_cont, lv_disp_get_hor_res( NULL ) - 10, 60);
    lv_obj_add_style( autoconnect_cont, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_obj_align( autoconnect_cont, toggle_leds_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    autoconnect_onoff = lv_switch_create( autoconnect_cont, NULL );
    lv_obj_add_protect( autoconnect_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( autoconnect_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( autoconnect_onoff, LV_ANIM_ON );
    lv_obj_align( autoconnect_onoff, autoconnect_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( autoconnect_onoff, toggle_autoconnect_onoff_event_handler );
    lv_obj_t *autoconnect_label = lv_label_create( autoconnect_cont, NULL);
    lv_obj_add_style( autoconnect_label, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_label_set_text( autoconnect_label, "connect to wheel\nautomatically");
    lv_obj_align( autoconnect_label, autoconnect_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *horn_press_cont = lv_obj_create( wheel_page, NULL );
    lv_page_glue_obj(autoconnect_cont, true);
    lv_obj_set_size(horn_press_cont, lv_disp_get_hor_res( NULL ) - 10, 60);
    lv_obj_add_style( horn_press_cont, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_obj_align( horn_press_cont, autoconnect_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    horn_press_onoff = lv_switch_create( horn_press_cont, NULL );
    lv_obj_add_protect( horn_press_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( horn_press_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( horn_press_onoff, LV_ANIM_ON );
    lv_obj_align( horn_press_onoff, horn_press_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( horn_press_onoff, toggle_horn_onoff_event_handler );
    lv_obj_t *horn_press_label = lv_label_create( horn_press_cont, NULL);
    lv_obj_add_style( horn_press_label, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_label_set_text( horn_press_label, "sound horn on\npress at > 3kmh");
    lv_obj_align( horn_press_label, horn_press_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    if ( wheelctl_get_config( WHEELCTL_CONFIG_LIGHTS_OFF ) )
        lv_switch_on( disable_startuplights_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( disable_startuplights_onoff, LV_ANIM_OFF );

    if ( wheelctl_get_config( WHEELCTL_CONFIG_LED ) )
        lv_switch_on( toggle_leds_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( toggle_leds_onoff, LV_ANIM_OFF );

    if ( wheelctl_get_config( WHEELCTL_CONFIG_HORN ) )
        lv_switch_on( horn_press_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( horn_press_onoff, LV_ANIM_OFF );

    if ( blectl_get_autoconnect() )
        lv_switch_on( autoconnect_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( autoconnect_onoff, LV_ANIM_OFF );
}

static void enter_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       wheel_settings_tile_setup();
                                        mainbar_jump_to_tilenumber( wheel_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( eucdash_get_tile_num(), LV_ANIM_OFF );
                                        wheelctl_save_config();
                                        break;
    }
}

static void disable_startuplights_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  wheelctl_set_config(WHEELCTL_CONFIG_LIGHTS_OFF, lv_switch_get_state( obj ));
    }
}
static void toggle_leds_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  wheelctl_set_config(WHEELCTL_CONFIG_LED, lv_switch_get_state( obj ));
    }
}
static void toggle_horn_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  wheelctl_set_config(WHEELCTL_CONFIG_HORN, lv_switch_get_state( obj ));
    }
}
static void toggle_autoconnect_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  blectl_set_autoconnect(lv_switch_get_state( obj ));
    }
}
