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

lv_obj_t *wheelscan_tile=NULL;
lv_style_t wheelscan_style;
lv_style_t wheelscan_heading_style;
lv_style_t wheelscan_data_style;
lv_style_t wheelscan_list_style;
//uint32_t wheelscan_tile_num;

lv_obj_t *wheelscan_list=NULL;

lv_obj_t *wheelscan_label=NULL;
lv_obj_t *wheelscan_ssid=NULL;
lv_obj_t * wheelscan_mbox=NULL;
lv_obj_t * wheelscan_list_btn=NULL;

int wheel_detect_id;
bool autoconnect_enabled;

//static void enter_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event );
void wheelscan_select_event_cb( lv_obj_t * obj, lv_event_t event );
void wheelscan_create_list(bool scanning);
void wheelscan_create_list_btn();
bool wheelscan_blectl_event_cb(EventBits_t event, void *arg);
void wheelscan_refresh_list(void);

LV_IMG_DECLARE(exit_32px);

void wheelscan_tile_setup( bool ac ) {
    // get an app tile and copy mainstyle
    autoconnect_enabled = ac;
    if (autoconnect_enabled) {
        //blectl_set_autoconnect(false);
    }
    wheelscan_tile_num = setup_get_submenu2_tile_num();
    wheelscan_tile = mainbar_get_tile_obj( wheelscan_tile_num );
    lv_obj_clean(wheelscan_tile);
    lv_style_copy( &wheelscan_style, mainbar_get_style() );
    lv_style_set_bg_color( &wheelscan_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK );
    lv_style_set_bg_opa( &wheelscan_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wheelscan_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &wheelscan_heading_style, &wheelscan_style );
    lv_style_set_text_color( &wheelscan_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );
    lv_style_copy( &wheelscan_data_style, &wheelscan_style );
    lv_style_set_text_color( &wheelscan_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );

    lv_obj_add_style( wheelscan_tile, LV_OBJ_PART_MAIN, &wheelscan_style );

    lv_obj_t *exit_btn = lv_imgbtn_create( wheelscan_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &wheelscan_style );
    lv_obj_align( exit_btn, wheelscan_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_wheelscan_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( wheelscan_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &wheelscan_heading_style );
    lv_label_set_text( exit_label, "detected wheels");
    lv_obj_align( exit_label, exit_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0 );

    wheelscan_create_list(true);

    blectl_set_autoconnect(true);
    blectl_scan_once(2);

    blectl_register_cb(BLECTL_CLI_DETECT_DONE, wheelscan_blectl_event_cb, "wheelscan");
}

void wheelscan_create_list(bool scanning) {
    wheelscan_list = lv_list_create( wheelscan_tile, NULL);
    lv_obj_set_size( wheelscan_list, lv_disp_get_hor_res( NULL ), 190);
    lv_style_init( &wheelscan_list_style  );
    lv_style_set_border_width( &wheelscan_list_style , LV_OBJ_PART_MAIN, 0);
    lv_style_set_radius( &wheelscan_list_style , LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( wheelscan_list, LV_OBJ_PART_MAIN, &wheelscan_list_style  );
    lv_obj_align( wheelscan_list, wheelscan_tile, LV_ALIGN_IN_TOP_MID, 0, 50);

    if (scanning) {
        lv_obj_t *scan_label = lv_label_create( wheelscan_list, NULL);
        lv_label_set_text( scan_label, "scanning...");
        lv_obj_align( scan_label, NULL, LV_ALIGN_CENTER, 0, 0 );
    }
}


void wheelscan_create_list_btn() {
    Serial.println("creating list items..");
    for( int i = 0 ; i < MAX_DETECTED_WHEELS ; i++ ) {
        wheelscan_list_btn = lv_list_add_btn(wheelscan_list, NULL, NULL);
        Serial.printf("adding wheel %d\n", i);
        lv_btn_set_layout(wheelscan_list_btn, LV_LAYOUT_OFF);
        std::stringstream wn_string;
        wn_string << i+1;
        lv_obj_t * wheeladdress_label = lv_label_create(wheelscan_list_btn, NULL);
        lv_label_set_text(wheeladdress_label, blectl_get_detected_wheel_address(i).c_str());
        lv_obj_align( wheeladdress_label, wheelscan_list_btn, LV_ALIGN_IN_RIGHT_MID, -15, 0);
        lv_obj_t * wheelnum_label = lv_label_create(wheelscan_list_btn, NULL);
        lv_label_set_text(wheelnum_label, wn_string.str().c_str());
        lv_obj_align( wheelnum_label, wheelscan_list_btn, LV_ALIGN_IN_LEFT_MID , 10, 6);
        lv_obj_t * wheeltype_label = lv_label_create(wheelscan_list_btn, NULL);
        lv_label_set_text(wheeltype_label, blectl_get_detected_wheel_name(i).c_str());
        lv_obj_align( wheeltype_label, wheelnum_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_obj_set_event_cb( wheelscan_list_btn, wheelscan_select_event_cb);
    }   
}

bool wheelscan_blectl_event_cb(EventBits_t event, void *arg) {
    switch( event ) {
        case BLECTL_CLI_DETECT_DONE:
            blectl_set_autoconnect(false);
            wheelscan_refresh_list();
            break;
    }
    return( true );
}

void wheelscan_refresh_list() {
    Serial.println("refreshing stored wheel list (cleaning list)");
    lv_obj_del(wheelscan_list);
    Serial.println("refreshing stored wheel list (reloading)");
    wheelscan_create_list(false);
    wheelscan_create_list_btn();
}

static void exit_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_submenu_tile_num(), LV_ANIM_OFF );
        blectl_clear_event(BLECTL_CLI_DETECT_DONE);
        blectl_clear_event(BLECTL_CLI_DETECT);
        if (autoconnect_enabled) blectl_set_autoconnect(true);
                                        break;
    }
}

void wheelscan_mbox_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (lv_msgbox_get_active_btn(obj) == 0) {
            printf("store wheel to slot %d\n", blectl_get_free_wheelslot());
            blectl_add_stored_wheel(blectl_get_detected_wheel_address(wheel_detect_id), blectl_get_detected_wheel_type(wheel_detect_id), blectl_get_free_wheelslot());
            lv_obj_del(wheelscan_mbox);
            mainbar_jump_to_tilenumber( setup_get_submenu_tile_num(), LV_ANIM_OFF );
            blectl_clear_event(BLECTL_CLI_DETECT_DONE);
            if (autoconnect_enabled) blectl_set_autoconnect(true);
            blectl_reset_scandelay();
        }
        
        if (lv_msgbox_get_active_btn(obj) == 1) lv_obj_del(wheelscan_mbox);
    }
}

void wheelscan_select_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):   static const char * wheelscan_popup_btns[] = {"store", "cancel", ""};
                                    wheelscan_mbox = lv_msgbox_create(wheelscan_tile, NULL);
                                    //lv_style_t mbox_style;
                                    //lv_style_init(&mbox_style);
                                    //lv_style_set_radius(&mbox_style, LV_OBJ_PART_MAIN, 1);
                                    wheel_detect_id = lv_list_get_btn_index(wheelscan_list, obj);
                                    
                                    std::stringstream btnindex_string;
                                    btnindex_string << wheel_detect_id+1;
                                    String detect_no = btnindex_string.str().c_str();
                                    String mbox_title = "selected wheel:\n#" + detect_no + " " + blectl_get_stored_wheel_name(wheel_detect_id) + " " + blectl_get_stored_wheel_address(wheel_detect_id);
                                    lv_msgbox_set_text(wheelscan_mbox, mbox_title.c_str());
                                    lv_msgbox_add_btns(wheelscan_mbox, wheelscan_popup_btns);
                                    //lv_obj_add_style(wheelscan_mbox, LV_MSGBOX_PART_BTN, &mbox_style);
                                    lv_obj_set_width(wheelscan_mbox, 230);
                                    lv_obj_set_event_cb(wheelscan_mbox, wheelscan_mbox_event_cb);
                                    lv_obj_align(wheelscan_mbox, NULL, LV_ALIGN_CENTER, 0, 0);
    }
}



