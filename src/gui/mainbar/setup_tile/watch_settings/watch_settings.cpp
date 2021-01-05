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

#include "gui/mainbar/setup_tile/bluetooth_settings/bluetooth_settings.h"
#include "gui/mainbar/setup_tile/battery_settings/battery_settings.h"
#include "gui/mainbar/setup_tile/battery_settings/battery_view.h"
#include "gui/mainbar/setup_tile/time_settings/time_settings.h"
#include "gui/mainbar/setup_tile/display_settings/display_settings.h"
#include "gui/mainbar/setup_tile/utilities/utilities.h"
#include "gui/mainbar/setup_tile/wlan_settings/wlan_settings.h"


lv_obj_t *watch_settings_tile=NULL;
lv_obj_t *watch_submenu_tile=NULL;
lv_style_t watch_settings_style;
lv_style_t watch_settings_page_style;
lv_style_t watch_settings_page_edge_style;
lv_style_t watch_settings_heading_style;
lv_style_t watch_settings_data_style;
uint32_t watch_tile_num;
uint32_t watch_submenu_tile_num;
int cont_height = 50;

lv_obj_t *watch_settings_page = NULL;


watch_settings_item_t *menu_item[MAX_MENU_ITEMS];

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(time_32px);
LV_IMG_DECLARE(brightness_32px);
LV_IMG_DECLARE(setup_32px);
LV_IMG_DECLARE(right_32px);
LV_IMG_DECLARE(watch_64px);

static void enter_watch_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_watch_setup_event_cb( lv_obj_t * obj, lv_event_t event );
void watch_settings_menu_item_setup();

void watch_settings_tile_setup( void ) {
    // set up watch settings tile
    log_i("add app tile");
    watch_tile_num = mainbar_add_app_tile( 1, 1, "watch settings" );
    watch_settings_tile = mainbar_get_tile_obj( watch_tile_num );
    // set up styles
    log_i("set up watch settings styles");
    lv_style_copy( &watch_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &watch_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &watch_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &watch_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &watch_settings_heading_style, &watch_settings_style );
    lv_style_set_text_color( &watch_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_style_copy( &watch_settings_data_style, &watch_settings_style );
    lv_style_set_text_color( &watch_settings_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );

    lv_obj_add_style( watch_settings_tile, LV_OBJ_PART_MAIN, &watch_settings_style );
    //add icon to setup screen
    icon_t *watch_setup_icon = setup_register( "watch setup", &watch_64px, enter_watch_setup_event_cb );
    setup_hide_indicator( watch_setup_icon );
    log_i("set up watch settings objects");
    //create top bar objects on tile
    log_i("set up watch settings top button");
    lv_obj_t *exit_btn = lv_imgbtn_create( watch_settings_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &watch_settings_style );
    lv_obj_align( exit_btn, watch_settings_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_watch_setup_event_cb );
    
    log_i("set up watch settings setup icon");
    lv_obj_t *exit_label = lv_label_create( watch_settings_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &watch_settings_heading_style  );
    lv_label_set_text( exit_label, "watch settings");
    lv_obj_align( exit_label, watch_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 15 );

    log_i("set up page");
    //create scrollable page for watch settings menu
    lv_style_copy( &watch_settings_page_style, &watch_settings_style );
    lv_style_set_pad_all(&watch_settings_page_style, LV_STATE_DEFAULT, 0);
    lv_style_copy( &watch_settings_page_edge_style, &watch_settings_page_style );
    lv_style_set_bg_color(&watch_settings_page_edge_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&watch_settings_page_edge_style, LV_STATE_DEFAULT, 30);
    watch_settings_page = lv_page_create( watch_settings_tile, NULL);
    lv_obj_set_size(watch_settings_page, lv_disp_get_hor_res( NULL ), 195);
    lv_page_set_edge_flash(watch_settings_page, true);
    lv_obj_add_style(watch_settings_page, LV_OBJ_PART_MAIN, &watch_settings_page_style );
    lv_obj_add_style(watch_settings_page, LV_PAGE_PART_EDGE_FLASH, &watch_settings_page_edge_style );
    lv_obj_add_style(watch_settings_page, LV_PAGE_PART_SCROLLBAR, &watch_settings_page_edge_style );
    lv_obj_align( watch_settings_page, watch_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 45 );
    log_i("set up menu items");

    //watch_submenu_tile_num = mainbar_add_app_tile( 1, 1, "watch submenu" );
    //watch_submenu_tile = mainbar_get_tile_obj( watch_submenu_tile_num );

    watch_settings_menu_item_setup();
}

uint32_t watch_get_tile_num(){
    return watch_tile_num;
}

uint32_t watch_get_submenu_tile_num(){
    return watch_submenu_tile_num;
}

uint32_t watch_settings_register_menu_item(const lv_img_dsc_t *icon, lv_event_cb_t event_cb, const char *item_label) {
    
    static int item_entries = 0;
    item_entries++;

    log_i("add item");
    watch_settings_item_t menu_item[ item_entries - 1 ];
    log_i("create menu cont");
    menu_item[ item_entries - 1 ].cont = lv_cont_create( watch_settings_page, NULL);
    lv_page_glue_obj(menu_item[ item_entries - 1 ].cont, true);
    lv_obj_set_size(menu_item[ item_entries - 1 ].cont, lv_disp_get_hor_res( NULL ) - 10, cont_height);
    lv_obj_add_style( menu_item[ item_entries - 1 ].cont, LV_OBJ_PART_MAIN, &watch_settings_style  );
    log_i("align menu cont");
    lv_obj_align( menu_item[ item_entries - 1 ].cont, NULL, LV_ALIGN_IN_TOP_MID, 0, cont_height * (item_entries - 1) );
    log_i("set event cb");
    lv_obj_set_event_cb( menu_item[ item_entries - 1 ].cont, event_cb );

    menu_item[ item_entries - 1 ].icon_cont = lv_cont_create(menu_item[ item_entries - 1 ].cont, NULL);
    lv_obj_set_size(menu_item[ item_entries - 1 ].icon_cont, 32, 32);
    lv_obj_add_style( menu_item[ item_entries - 1 ].icon_cont, LV_OBJ_PART_MAIN, &watch_settings_style  );
    lv_obj_align( menu_item[ item_entries - 1 ].icon_cont, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0 );

    menu_item[ item_entries - 1 ].icon = lv_img_create(menu_item[ item_entries - 1 ].icon_cont, NULL);
    lv_img_set_src(menu_item[ item_entries - 1 ].icon, icon);
    lv_obj_align( menu_item[ item_entries - 1 ].icon, NULL, LV_ALIGN_CENTER, 0, 0 );
    
    menu_item[ item_entries - 1 ].arrow = lv_img_create(menu_item[ item_entries - 1 ].cont, NULL);
    lv_img_set_src(menu_item[ item_entries - 1 ].arrow, &right_32px);
    lv_obj_align( menu_item[ item_entries - 1 ].arrow, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0 );
    
    log_i("create menu label");
    menu_item[ item_entries - 1 ].label = lv_label_create( menu_item[ item_entries - 1 ].cont, NULL);
    lv_obj_add_style( menu_item[ item_entries - 1 ].label, LV_OBJ_PART_MAIN, &watch_settings_heading_style );
    lv_label_set_text( menu_item[ item_entries - 1 ].label, item_label);
    lv_obj_align( menu_item[ item_entries - 1 ].label, NULL, LV_ALIGN_IN_LEFT_MID, 40, 0 );
    lv_obj_add_protect( menu_item[ item_entries - 1 ].label, LV_PROTECT_CLICK_FOCUS);
    return item_entries - 1;
}

void watch_settings_menu_item_setup() { //add menu items in the desired order
    display_settings_tile_pre_setup();
    log_i("battery settings");
    battery_view_tile_pre_setup();
    battery_settings_tile_pre_setup();
    log_i("time settings");
    time_settings_tile_pre_setup(); 
    wlan_setup_tile_pre_setup();
    wlan_connection_tile_pre_setup();
    log_i("ble settings");
    bluetooth_settings_tile_pre_setup();
    utilities_tile_pre_setup();
}

static void enter_watch_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( watch_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_watch_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}