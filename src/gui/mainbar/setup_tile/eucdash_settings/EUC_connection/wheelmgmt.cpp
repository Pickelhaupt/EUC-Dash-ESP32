
/****************************************************************************
 *   Jesper Ortlund 2021
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

#include <sstream>

lv_obj_t *wheelmgmt_tile=NULL;
lv_style_t wheelmgmt_style;
lv_style_t wheelmgmt_heading_style;
lv_style_t wheelmgmt_data_style;
lv_style_t wheelmgmt_list_style;
//uint32_t wheelmgmt_tile_num;

lv_obj_t *wheelmgmt_list=NULL;

lv_obj_t *wheelmgmt_label=NULL;
lv_obj_t *wheelmgmt_ssid=NULL;
lv_obj_t * wheelmgmt_mbox=NULL;
lv_obj_t * wheelmgmt_list_btn=NULL;

int wheel_slot_id;

//static void enter_wheelmgmt_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wheelmgmt_event_cb( lv_obj_t * obj, lv_event_t event );
static void wheelmgmt_select_event_cb( lv_obj_t * obj, lv_event_t event );
void wheelmgmt_create_list_btn();

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(unlock_16px);

void wheelmgmt_tile_setup( void ) {
    // get an app tile and copy mainstyle
    wheelmgmt_tile_num = setup_get_submenu2_tile_num();
    wheelmgmt_tile = mainbar_get_tile_obj( wheelmgmt_tile_num );
    lv_obj_clean(wheelmgmt_tile);
    lv_style_copy( &wheelmgmt_style, mainbar_get_style() );
    lv_style_set_bg_color( &wheelmgmt_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK );
    lv_style_set_bg_opa( &wheelmgmt_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wheelmgmt_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &wheelmgmt_heading_style, &wheelmgmt_style );
    lv_style_set_text_color( &wheelmgmt_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );
    lv_style_copy( &wheelmgmt_data_style, &wheelmgmt_style );
    lv_style_set_text_color( &wheelmgmt_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );

    lv_obj_add_style( wheelmgmt_tile, LV_OBJ_PART_MAIN, &wheelmgmt_style );

    lv_obj_t *exit_btn = lv_imgbtn_create( wheelmgmt_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &wheelmgmt_style );
    lv_obj_align( exit_btn, wheelmgmt_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_wheelmgmt_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( wheelmgmt_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &wheelmgmt_heading_style );
    lv_label_set_text( exit_label, "stored wheels");
    lv_obj_align( exit_label, exit_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0 );

    wheelmgmt_create_list_btn();           
}

void wheelmgmt_create_list_btn() {
    wheelmgmt_list = lv_list_create( wheelmgmt_tile, NULL);
    lv_obj_set_size( wheelmgmt_list, lv_disp_get_hor_res( NULL ), 190);
    lv_style_init( &wheelmgmt_list_style  );
    lv_style_set_border_width( &wheelmgmt_list_style , LV_OBJ_PART_MAIN, 0);
    lv_style_set_radius( &wheelmgmt_list_style , LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( wheelmgmt_list, LV_OBJ_PART_MAIN, &wheelmgmt_list_style  );
    lv_obj_align( wheelmgmt_list, wheelmgmt_tile, LV_ALIGN_IN_TOP_MID, 0, 50);
    
    Serial.println("creating list items..");
    for( int i = 0 ; i < MAX_STORED_WHEELS ; i++ ) {
        wheelmgmt_list_btn = lv_list_add_btn(wheelmgmt_list, NULL, NULL);
        Serial.printf("adding wheel %d\n", i);
        lv_btn_set_layout(wheelmgmt_list_btn, LV_LAYOUT_OFF);
        std::stringstream wn_string;
        wn_string << i+1;
        lv_obj_t * wheeladdress_label = lv_label_create(wheelmgmt_list_btn, NULL);
        lv_label_set_text(wheeladdress_label, blectl_get_stored_wheel_address(i).c_str());
        lv_obj_align( wheeladdress_label, wheelmgmt_list_btn, LV_ALIGN_IN_RIGHT_MID, -15, 0);
        lv_obj_t * wheelnum_label = lv_label_create(wheelmgmt_list_btn, NULL);
        lv_label_set_text(wheelnum_label, wn_string.str().c_str());
        lv_obj_align( wheelnum_label, wheelmgmt_list_btn, LV_ALIGN_IN_LEFT_MID , 10, 6);
        lv_obj_t * wheeltype_label = lv_label_create(wheelmgmt_list_btn, NULL);
        lv_label_set_text(wheeltype_label, blectl_get_stored_wheel_name(i).c_str());
        lv_obj_align( wheeltype_label, wheelnum_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_obj_set_event_cb( wheelmgmt_list_btn, wheelmgmt_select_event_cb);
    }   
}

void wheelmgmt_refresh_list(void) {
    Serial.println("refreshing stored wheel list (cleaning list)");
    lv_obj_del(wheelmgmt_list);
    Serial.println("refreshing stored wheel list (reloading)");
    wheelmgmt_create_list_btn();
}

static void exit_wheelmgmt_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_submenu_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

void wheelmgmt_mbox_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (lv_msgbox_get_active_btn(obj) == 0) {
            printf("delete wheel %d\n", wheel_slot_id);
            blectl_remove_stored_wheel(wheel_slot_id);
            lv_obj_del(wheelmgmt_mbox);
            wheelmgmt_refresh_list();
        }
        if (lv_msgbox_get_active_btn(obj) == 1) {
            printf("setting wheel %d to default\n", wheel_slot_id);
            blectl_set_prio_stored_wheel(wheel_slot_id);
            lv_obj_del(wheelmgmt_mbox);
            wheelmgmt_refresh_list();
        }
        if (lv_msgbox_get_active_btn(obj) == 2) lv_obj_del(wheelmgmt_mbox);
    }
}

void wheelmgmt_select_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):   static const char * wheelmgmt_popup_btns[] = {"delete", "default", "cancel", ""};
                                    wheelmgmt_mbox = lv_msgbox_create(wheelmgmt_tile, NULL);
                                    //lv_style_t mbox_style;
                                    //lv_style_init(&mbox_style);
                                    //lv_style_set_radius(&mbox_style, LV_OBJ_PART_MAIN, 1);
                                    wheel_slot_id = lv_list_get_btn_index(wheelmgmt_list, obj);
                                    
                                    std::stringstream btnindex_string;
                                    btnindex_string << wheel_slot_id+1;
                                    String slot_no = btnindex_string.str().c_str();
                                    String mbox_title = "selected wheel:\n#" + slot_no + " " + blectl_get_stored_wheel_name(wheel_slot_id) + " " + blectl_get_stored_wheel_address(wheel_slot_id);
                                    lv_msgbox_set_text(wheelmgmt_mbox, mbox_title.c_str());
                                    lv_msgbox_add_btns(wheelmgmt_mbox, wheelmgmt_popup_btns);
                                    //lv_obj_add_style(wheelmgmt_mbox, LV_MSGBOX_PART_BTN, &mbox_style);
                                    lv_obj_set_width(wheelmgmt_mbox, 230);
                                    lv_obj_set_event_cb(wheelmgmt_mbox, wheelmgmt_mbox_event_cb);
                                    lv_obj_align(wheelmgmt_mbox, NULL, LV_ALIGN_CENTER, 0, 0);
    }
}

