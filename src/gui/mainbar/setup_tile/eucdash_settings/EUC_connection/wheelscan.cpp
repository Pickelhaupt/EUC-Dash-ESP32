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

lv_obj_t *wheelscan_tile=NULL;
lv_style_t wheelscan_style;
lv_style_t wheelscan_heading_style;
lv_style_t wheelscan_data_style;
lv_style_t wheelscan_list_style;
//uint32_t wheelscan_tile_num;

lv_obj_t *wheelscan_list=NULL;

lv_obj_t *wheelscan_label=NULL;
lv_obj_t *wheelscan_ssid=NULL;

//static void enter_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event );

LV_IMG_DECLARE(exit_32px);

void wheelscan_tile_setup( void ) {
    // get an app tile and copy mainstyle
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
  
    wheelscan_list = lv_list_create( wheelscan_tile, NULL);
    lv_obj_set_size( wheelscan_list, lv_disp_get_hor_res( NULL ), 190);
    lv_style_init( &wheelscan_list_style  );
    lv_style_set_border_width( &wheelscan_list_style , LV_OBJ_PART_MAIN, 0);
    lv_style_set_radius( &wheelscan_list_style , LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( wheelscan_list, LV_OBJ_PART_MAIN, &wheelscan_list_style  );
    lv_obj_align( wheelscan_list, wheelscan_tile, LV_ALIGN_IN_TOP_MID, 0, 50);

}



static void exit_wheelscan_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_submenu_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

