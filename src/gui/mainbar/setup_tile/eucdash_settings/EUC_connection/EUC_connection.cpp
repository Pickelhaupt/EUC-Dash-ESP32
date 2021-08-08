/****************************************************************************
 *   2021 Jesper Ortlund  
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
#include "EUC_connection.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/keyboard.h"
#include "gui/setup.h"

#include "hardware/blectl.h"
#include "hardware/json_psram_allocator.h"
#include "gui/mainbar/setup_tile/eucdash_settings/eucdash_settings.h"


lv_obj_t *euc_connection_tile=NULL;
lv_style_t euc_connection_style;
lv_style_t euc_connection_heading_style;
lv_style_t euc_connection_data_style;
lv_style_t euc_list_style;
uint32_t euc_connection_tile_num;

uint32_t wheelscan_tile_num;
uint32_t wheelmgmt_tile_num;

lv_obj_t *euc_add_tile=NULL;
lv_style_t euc_add_style;
lv_obj_t *euc_add_name_label=NULL;
lv_obj_t *euc_add_pass_textfield=NULL;
uint32_t euc_add_tile_num;

lv_obj_t *euc_connect_onoff=NULL;
lv_obj_t *euc_address_list=NULL;

lv_obj_t *euc_connection_label=NULL;
lv_obj_t *euc_connection_ssid=NULL;

static void enter_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event );
static void enter_wheelmgmt_event_cb( lv_obj_t * obj, lv_event_t event );

static void enter_euc_connection_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_euc_connection_event_cb( lv_obj_t * obj, lv_event_t event );
static void euc_connect_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
void euc_connection_enter_pass_event_cb( lv_obj_t * obj, lv_event_t event );
bool euc_setup_eucctl_event_cb( EventBits_t event, void *arg );

LV_IMG_DECLARE(lock_16px);
LV_IMG_DECLARE(unlock_16px);
LV_IMG_DECLARE(check_32px);
LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(trash_32px);
LV_IMG_DECLARE(wheel_32px);
LV_IMG_DECLARE(eucdash_64px);

void euc_connection_tile_pre_setup( void ){
    eucdash_settings_register_menu_item(&wheel_32px, enter_euc_connection_event_cb, "wheel connection");
}

void euc_connection_tile_setup( void ) {
    // get an app tile and copy mainstyle
    euc_connection_tile_num = setup_get_submenu_tile_num();
    euc_add_tile_num = setup_get_submenu2_tile_num();
    euc_connection_tile = mainbar_get_tile_obj( euc_connection_tile_num );
    lv_obj_clean(euc_connection_tile);
    lv_style_copy( &euc_connection_style, mainbar_get_style() );
    lv_style_set_bg_color( &euc_connection_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK );
    lv_style_set_bg_opa( &euc_connection_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &euc_connection_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &euc_connection_heading_style, &euc_connection_style );
    lv_style_set_text_color( &euc_connection_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );
    lv_style_copy( &euc_connection_data_style, &euc_connection_style );
    lv_style_set_text_color( &euc_connection_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );

    lv_obj_add_style( euc_connection_tile, LV_OBJ_PART_MAIN, &euc_connection_style );

    lv_obj_t *exit_btn = lv_imgbtn_create( euc_connection_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &euc_connection_style );
    lv_obj_align( exit_btn, euc_connection_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_euc_connection_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( euc_connection_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &euc_connection_heading_style );
    lv_label_set_text( exit_label, "wheel connection");
    lv_obj_align( exit_label, exit_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0 );

    lv_obj_t *autoconnect_label = lv_label_create( euc_connection_tile, NULL);
    lv_obj_add_style( autoconnect_label, LV_OBJ_PART_MAIN, &euc_connection_style );
    lv_label_set_text( autoconnect_label, "scan for wheels");
    lv_obj_align( autoconnect_label, euc_connection_tile, LV_ALIGN_IN_TOP_LEFT, 10, 50 );    
  
    euc_connect_onoff = lv_switch_create( euc_connection_tile, NULL );
    lv_obj_add_protect( euc_connect_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( euc_connect_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style()  );
    lv_obj_add_style( euc_connect_onoff, LV_SLIDER_PART_KNOB, mainbar_get_knob_style() );
    if(blectl_get_event(BLECTL_CLI_DETECT)) lv_switch_on( euc_connect_onoff, LV_ANIM_ON );
    else lv_switch_off( euc_connect_onoff, LV_ANIM_ON );
    lv_obj_align( euc_connect_onoff, euc_connection_tile, LV_ALIGN_IN_TOP_RIGHT, -10, 50 );
    lv_obj_set_event_cb( euc_connect_onoff, euc_connect_onoff_event_handler);

    lv_obj_t *wheelmgmt_label = lv_label_create( euc_connection_tile, NULL);
    lv_obj_add_style( wheelmgmt_label, LV_OBJ_PART_MAIN, &euc_connection_heading_style );
    lv_obj_set_width(wheelmgmt_label, 100);
    lv_label_set_align(wheelmgmt_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text( wheelmgmt_label, "manage stored\nwheels");
    lv_obj_align( wheelmgmt_label, euc_connection_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );
    
    lv_obj_t *wheelmgmt_btn = lv_imgbtn_create( euc_connection_tile, NULL);
    lv_imgbtn_set_src( wheelmgmt_btn, LV_BTN_STATE_RELEASED, &eucdash_64px);
    lv_imgbtn_set_src( wheelmgmt_btn, LV_BTN_STATE_PRESSED, &eucdash_64px);
    lv_imgbtn_set_src( wheelmgmt_btn, LV_BTN_STATE_CHECKED_RELEASED, &eucdash_64px);
    lv_imgbtn_set_src( wheelmgmt_btn, LV_BTN_STATE_CHECKED_PRESSED, &eucdash_64px);
    lv_obj_add_style( wheelmgmt_btn, LV_IMGBTN_PART_MAIN, &euc_connection_style );
    lv_obj_align( wheelmgmt_btn, wheelmgmt_label, LV_ALIGN_OUT_TOP_MID, 0, 0 );
    lv_obj_set_event_cb( wheelmgmt_btn, enter_wheelmgmt_event_cb );

    lv_obj_t *wheelscan_label = lv_label_create( euc_connection_tile, NULL);
    lv_obj_add_style( wheelscan_label, LV_OBJ_PART_MAIN, &euc_connection_heading_style );
    lv_obj_set_width(wheelscan_label, 100);
    lv_label_set_align(wheelscan_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text( wheelscan_label, "scan for\nnew wheels");
    lv_obj_align( wheelscan_label, euc_connection_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10 );

    lv_obj_t *wheelscan_btn = lv_imgbtn_create( euc_connection_tile, NULL);
    lv_imgbtn_set_src( wheelscan_btn, LV_BTN_STATE_RELEASED, &eucdash_64px);
    lv_imgbtn_set_src( wheelscan_btn, LV_BTN_STATE_PRESSED, &eucdash_64px);
    lv_imgbtn_set_src( wheelscan_btn, LV_BTN_STATE_CHECKED_RELEASED, &eucdash_64px);
    lv_imgbtn_set_src( wheelscan_btn, LV_BTN_STATE_CHECKED_PRESSED, &eucdash_64px);
    lv_obj_add_style( wheelscan_btn, LV_IMGBTN_PART_MAIN, &euc_connection_style );
    lv_obj_align( wheelscan_btn, wheelscan_label, LV_ALIGN_OUT_TOP_MID, 0, 0 );
    lv_obj_set_event_cb( wheelscan_btn, enter_wheelscan_event_cb );
}

static void enter_euc_connection_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       euc_connection_tile_setup();
                                        mainbar_jump_to_tilenumber( euc_connection_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_euc_connection_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( eucdash_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

static void enter_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       wheelscan_tile_setup();
                                        blectl_set_event(BLECTL_CLI_DETECT);
                                        mainbar_jump_to_tilenumber( wheelscan_tile_num, LV_ANIM_OFF );
                                        blectl_reset_scandelay();
                                        break;
    }
}

static void enter_wheelmgmt_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       wheelmgmt_tile_setup();
                                        mainbar_jump_to_tilenumber( wheelmgmt_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void euc_connect_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED ): if(lv_switch_get_state( obj )) blectl_set_event(BLECTL_CLI_DETECT);
    }
}