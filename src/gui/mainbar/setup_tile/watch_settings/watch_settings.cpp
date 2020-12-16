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
#include "watch_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
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
lv_obj_t *watch_settings_page = NULL;


watch_settings_item_t *menu_item[MAX_MENU_ITEMS];

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(wheel_64px);

static void enter_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void disable_startuplights_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_leds_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_horn_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void toggle_autoconnect_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void item_1_event_cb(lv_obj_t * obj, lv_event_t event);
static void item_2_event_cb(lv_obj_t * obj, lv_event_t event);


void watch_settings_tile_setup( void ) {
    // get an app tile and copy mainstyle
    wheel_tile_num = mainbar_add_app_tile( 1, 1, "watch settings" );
    wheel_settings_tile = mainbar_get_tile_obj( wheel_tile_num );
    lv_style_copy( &wheel_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &wheel_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &wheel_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wheel_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &wheel_settings_heading_style, &wheel_settings_style );
    lv_style_set_text_color( &wheel_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_obj_add_style( wheel_settings_tile, LV_OBJ_PART_MAIN, &wheel_settings_style );

    icon_t *wheel_setup_icon = setup_register( "wheel", &wheel_64px, enter_wheel_setup_event_cb );
    setup_hide_indicator( wheel_setup_icon );

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
    lv_label_set_text( exit_label, "watch settings");
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

    watch_settings_menu_item_setup();
}

uint32_t watch_settings_register_menu_item(const char *item_name, const lv_img_dsc_t *icon, lv_event_cb_t event_cb, const char *item_label) {
    int item_entries = 0;
    item_entries++;

    watch_settings_item_t menu_item[ item_entries - 1 ];
    menu_item[ item_entries - 1 ].id = item_name;
    menu_item[ item_entries - 1 ].item_event_cb = event_cb;

    menu_item[ item_entries - 1 ].cont = lv_cont_create( watch_settings_page, NULL);
    lv_obj_set_size(menu_item[item_entries - 1].cont, lv_disp_get_hor_res( NULL ) - 10, 60);
    if( item_entries == 1) {
        lv_obj_align( menu_item[ item_entries - 1 ].cont, watch_settings_page, LV_ALIGN_IN_TOP_LEFT, 0, 0 );
    }
    else {
        lv_obj_align( menu_item[ item_entries - 1 ].cont, menu_item[ item_entries - 2 ].cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    }
    lv_obj_set_event_cb( menu_item[ item_entries - 1 ].cont, event_cb );
    menu_item[ item_entries - 1 ].label = lv_label_create( menu_item[ item_entries - 1 ].cont, NULL);
    lv_obj_add_style( menu_item[ item_entries - 1 ].label, LV_OBJ_PART_MAIN, &wheel_settings_style  );
    lv_label_set_text( menu_item[ item_entries - 1 ].label, item_label);
    lv_obj_align( menu_item[ item_entries - 1 ].label, menu_item[ item_entries - 1 ].cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
}


void watch_settings_menu_item_setup() {
    watch_settings_register_menu_item("item 1", &exit_32px, item_1_event_cb, "menu item 1");
    watch_settings_register_menu_item("item 2", &exit_32px, item_2_event_cb, "menu item 2");
}

static void item_1_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):      log_i("item 1 clicked");
                                        break;
    }
}

static void item_2_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):      log_i("item 2 clicked");
                                        break;
    }
}

static void enter_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( wheel_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_wheel_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_tile_num(), LV_ANIM_OFF );
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
