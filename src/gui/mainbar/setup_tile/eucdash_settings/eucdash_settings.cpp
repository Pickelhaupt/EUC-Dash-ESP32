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
#include "eucdash_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/setup.h"

#include "hardware/bma.h"
#include "hardware/blectl.h"
#include "hardware/motor.h"
#include "hardware/wheelctl.h"

#include "gui/mainbar/setup_tile/eucdash_settings/wheel_settings/wheel_settings.h"
#include "gui/mainbar/setup_tile/eucdash_settings/dashboard_settings/dashboard_settings.h"
#include "gui/mainbar/setup_tile/eucdash_settings/EUC_connection/EUC_connection.h"

lv_obj_t *eucdash_settings_tile=NULL;
lv_obj_t *eucdash_submenu_tile=NULL;
lv_style_t eucdash_settings_style;
lv_style_t eucdash_settings_page_style;
lv_style_t eucdash_settings_page_edge_style;
lv_style_t eucdash_settings_heading_style;
lv_style_t eucdash_settings_data_style;
uint32_t eucdash_tile_num;
uint32_t eucdash_submenu_tile_num;
int eucdash_cont_height = 50;

lv_obj_t *eucdash_settings_page = NULL;

eucdash_settings_item_t *eucdash_menu_item[MAX_MENU_ITEMS];

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(time_32px);
LV_IMG_DECLARE(brightness_32px);
LV_IMG_DECLARE(setup_32px);
LV_IMG_DECLARE(right_32px);
LV_IMG_DECLARE(eucdash_64px);

static void enter_eucdash_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_eucdash_setup_event_cb( lv_obj_t * obj, lv_event_t event );
void eucdash_settings_menu_item_setup();

void eucdash_settings_tile_setup( void ) {
    // set up eucdash settings tile
    log_i("add app tile");
    eucdash_tile_num = mainbar_add_app_tile( 1, 1, "eucdash settings" );
    eucdash_settings_tile = mainbar_get_tile_obj( eucdash_tile_num );
    // set up styles
    log_i("set up eucdash settings styles");
    lv_style_copy( &eucdash_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &eucdash_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &eucdash_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &eucdash_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &eucdash_settings_heading_style, &eucdash_settings_style );
    lv_style_set_text_color( &eucdash_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_style_copy( &eucdash_settings_data_style, &eucdash_settings_style );
    lv_style_set_text_color( &eucdash_settings_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );

    lv_obj_add_style( eucdash_settings_tile, LV_OBJ_PART_MAIN, &eucdash_settings_style );
    //add icon to setup screen
    icon_t *eucdash_setup_icon = setup_register( "eucdash setup", &eucdash_64px, enter_eucdash_setup_event_cb );
    setup_hide_indicator( eucdash_setup_icon );
    log_i("set up eucdash settings objects");
    //create top bar objects on tile
    log_i("set up eucdash settings top button");
    lv_obj_t *exit_btn = lv_imgbtn_create( eucdash_settings_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &eucdash_settings_style );
    lv_obj_align( exit_btn, eucdash_settings_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_eucdash_setup_event_cb );
    
    log_i("set up eucdash settings setup icon");
    lv_obj_t *exit_label = lv_label_create( eucdash_settings_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &eucdash_settings_heading_style  );
    lv_label_set_text( exit_label, "eucdash settings");
    lv_obj_align( exit_label, eucdash_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 15 );

    log_i("set up page");
    //create scrollable page for eucdash settings menu
    lv_style_copy( &eucdash_settings_page_style, &eucdash_settings_style );
    lv_style_set_pad_all(&eucdash_settings_page_style, LV_STATE_DEFAULT, 0);
    lv_style_copy( &eucdash_settings_page_edge_style, &eucdash_settings_page_style );
    lv_style_set_bg_color(&eucdash_settings_page_edge_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&eucdash_settings_page_edge_style, LV_STATE_DEFAULT, 30);
    eucdash_settings_page = lv_page_create( eucdash_settings_tile, NULL);
    lv_obj_set_size(eucdash_settings_page, lv_disp_get_hor_res( NULL ), 195);
    lv_page_set_edge_flash(eucdash_settings_page, true);
    lv_obj_add_style(eucdash_settings_page, LV_OBJ_PART_MAIN, &eucdash_settings_page_style );
    lv_obj_add_style(eucdash_settings_page, LV_PAGE_PART_EDGE_FLASH, &eucdash_settings_page_edge_style );
    lv_obj_add_style(eucdash_settings_page, LV_PAGE_PART_SCROLLBAR, &eucdash_settings_page_edge_style );
    lv_obj_align( eucdash_settings_page, eucdash_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 45 );
    log_i("set up menu items");

    //eucdash_submenu_tile_num = mainbar_add_app_tile( 1, 1, "eucdash submenu" );
    //eucdash_submenu_tile = mainbar_get_tile_obj( eucdash_submenu_tile_num );

    eucdash_settings_menu_item_setup();
}

uint32_t eucdash_get_tile_num(){
    return eucdash_tile_num;
}

uint32_t eucdash_get_submenu_tile_num(){
    return eucdash_submenu_tile_num;
}

uint32_t eucdash_settings_register_menu_item(const lv_img_dsc_t *icon, lv_event_cb_t event_cb, const char *item_label) {
    
    static int item_entries = 0;
    item_entries++;

    log_i("add item");
    eucdash_settings_item_t eucdash_menu_item[ item_entries - 1 ];
    log_i("create menu cont");
    eucdash_menu_item[ item_entries - 1 ].cont = lv_cont_create( eucdash_settings_page, NULL);
    lv_page_glue_obj(eucdash_menu_item[ item_entries - 1 ].cont, true);
    lv_obj_set_size(eucdash_menu_item[ item_entries - 1 ].cont, lv_disp_get_hor_res( NULL ) - 10, eucdash_cont_height);
    lv_obj_add_style( eucdash_menu_item[ item_entries - 1 ].cont, LV_OBJ_PART_MAIN, &eucdash_settings_style  );
    log_i("align menu cont");
    lv_obj_align( eucdash_menu_item[ item_entries - 1 ].cont, NULL, LV_ALIGN_IN_TOP_MID, 0, eucdash_cont_height * (item_entries - 1) );
    log_i("set event cb");
    lv_obj_set_event_cb( eucdash_menu_item[ item_entries - 1 ].cont, event_cb );
    
    eucdash_menu_item[ item_entries - 1 ].icon = lv_img_create(eucdash_menu_item[ item_entries - 1 ].cont, NULL);
    lv_img_set_src(eucdash_menu_item[ item_entries - 1 ].icon, icon);
    lv_obj_align( eucdash_menu_item[ item_entries - 1 ].icon, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0 );
    
    eucdash_menu_item[ item_entries - 1 ].arrow = lv_img_create(eucdash_menu_item[ item_entries - 1 ].cont, NULL);
    lv_img_set_src(eucdash_menu_item[ item_entries - 1 ].arrow, &right_32px);
    lv_obj_align( eucdash_menu_item[ item_entries - 1 ].arrow, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0 );
    
    log_i("create menu label");
    eucdash_menu_item[ item_entries - 1 ].label = lv_label_create( eucdash_menu_item[ item_entries - 1 ].cont, NULL);
    lv_obj_add_style( eucdash_menu_item[ item_entries - 1 ].label, LV_OBJ_PART_MAIN, &eucdash_settings_heading_style );
    lv_label_set_text( eucdash_menu_item[ item_entries - 1 ].label, item_label);
    lv_obj_align( eucdash_menu_item[ item_entries - 1 ].label, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0 );
    lv_obj_add_protect( eucdash_menu_item[ item_entries - 1 ].label, LV_PROTECT_CLICK_FOCUS);
    return item_entries - 1;
}

void eucdash_settings_menu_item_setup() { 
    log_i("wheel settings");
    wheel_settings_tile_pre_setup();
    dashboard_settings_tile_pre_setup();
    euc_connection_tile_pre_setup();
}

static void enter_eucdash_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( eucdash_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_eucdash_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}
