/****************************************************************************
 *   Tu May 22 21:23:51 2020
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
#include "move_settings.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/setup_tile.h"
#include "gui/setup.h"

#include "hardware/bma.h"
#include "hardware/motor.h"

lv_obj_t *move_settings_tile=NULL;
lv_style_t move_settings_style;
lv_style_t move_page_style;
lv_style_t move_page_edge_style;
lv_style_t move_settings_heading_style;
uint32_t move_tile_num;

lv_obj_t *stepcounter_onoff=NULL;
//lv_obj_t *doubleclick_onoff=NULL;
//lv_obj_t *tilt_onoff=NULL;
lv_obj_t *daily_stepcounter_onoff=NULL;
lv_obj_t *one_onoff=NULL;
lv_obj_t *two_onoff=NULL;
lv_obj_t *three_onoff=NULL;
lv_obj_t *four_onoff=NULL;
lv_obj_t *five_onoff=NULL;
lv_obj_t *six_onoff=NULL;
lv_obj_t *seven_onoff=NULL;
lv_obj_t *eight_onoff=NULL;

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(wheel_64px);

static void enter_move_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_move_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void stepcounter_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
//static void doubleclick_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
//static void tilt_onoff_event_handler(lv_obj_t * obj, lv_event_t event);
static void daily_stepcounter_onoff_event_handler(lv_obj_t * obj, lv_event_t event);

void move_settings_tile_setup( void ) {
    // get an app tile and copy mainstyle
    move_tile_num = mainbar_add_app_tile( 1, 1, "move settings" );
    move_settings_tile = mainbar_get_tile_obj( move_tile_num );
    lv_style_copy( &move_settings_style, mainbar_get_style() );
    lv_style_set_bg_color( &move_settings_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &move_settings_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &move_settings_style, LV_OBJ_PART_MAIN, 0);

    lv_style_copy( &move_settings_heading_style, &move_settings_style );
    lv_style_set_text_color( &move_settings_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_obj_add_style( move_settings_tile, LV_OBJ_PART_MAIN, &move_settings_style );

    icon_t *move_setup_icon = setup_register( "move", &wheel_64px, enter_move_setup_event_cb );
    setup_hide_indicator( move_setup_icon );

    lv_obj_t *exit_btn = lv_imgbtn_create( move_settings_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &move_settings_style );
    lv_obj_align( exit_btn, move_settings_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_move_setup_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( move_settings_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &move_settings_heading_style  );
    lv_label_set_text( exit_label, "wheel settings");
    lv_obj_align( exit_label, move_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 15 );

    lv_style_copy( &move_page_style, &move_settings_style );
    lv_style_set_pad_all(&move_page_style, LV_STATE_DEFAULT, 0);
    lv_style_copy( &move_page_edge_style, &move_page_style );
    lv_style_set_bg_color(&move_page_edge_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_opa(&move_page_edge_style, LV_STATE_DEFAULT, 30);
    lv_obj_t *move_page = lv_page_create( move_settings_tile, NULL);
    lv_obj_set_size(move_page, lv_disp_get_hor_res( NULL ), 195);
    //lv_page_set_scrl_width(move_page, 215);
    lv_page_set_edge_flash(move_page, true);
    lv_obj_add_style(move_page, LV_OBJ_PART_MAIN, &move_page_style );
    lv_obj_add_style(move_page, LV_PAGE_PART_EDGE_FLASH, &move_page_edge_style );
    lv_obj_align( move_page, move_settings_tile, LV_ALIGN_IN_TOP_MID, 0, 45 );

    lv_obj_t *stepcounter_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(stepcounter_cont, true);
    lv_obj_set_size(stepcounter_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( stepcounter_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( stepcounter_cont, move_page, LV_ALIGN_IN_TOP_LEFT, 0, 0 );
    stepcounter_onoff = lv_switch_create( stepcounter_cont, NULL );
    lv_obj_add_protect( stepcounter_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( stepcounter_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( stepcounter_onoff, LV_ANIM_ON );
    lv_obj_align( stepcounter_onoff, stepcounter_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( stepcounter_onoff, stepcounter_onoff_event_handler );
    lv_obj_t *stepcounter_label = lv_label_create( stepcounter_cont, NULL);
    lv_obj_add_style( stepcounter_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( stepcounter_label, "not implemented");
    lv_obj_align( stepcounter_label, stepcounter_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

/*
    lv_obj_t *doubleclick_cont = lv_obj_create( move_settings_tile, NULL );
    lv_obj_set_size(doubleclick_cont, lv_disp_get_hor_res( NULL ) , 50);
    lv_obj_add_style( doubleclick_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( doubleclick_cont, stepcounter_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    doubleclick_onoff = lv_switch_create( doubleclick_cont, NULL );
    lv_obj_add_protect( doubleclick_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( doubleclick_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( doubleclick_onoff, LV_ANIM_ON );
    lv_obj_align( doubleclick_onoff, doubleclick_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( doubleclick_onoff, doubleclick_onoff_event_handler );
    lv_obj_t *doubleclick_label = lv_label_create( doubleclick_cont, NULL);
    lv_obj_add_style( doubleclick_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( doubleclick_label, "doubletap to wake");
    lv_obj_align( doubleclick_label, doubleclick_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *tilt_cont = lv_obj_create( move_settings_tile, NULL );
    lv_obj_set_size(tilt_cont, lv_disp_get_hor_res( NULL ) , 50);
    lv_obj_add_style( tilt_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( tilt_cont, doubleclick_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    tilt_onoff = lv_switch_create( tilt_cont, NULL );
    lv_obj_add_protect( tilt_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( tilt_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( tilt_onoff, LV_ANIM_ON );
    lv_obj_align( tilt_onoff, tilt_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( tilt_onoff, tilt_onoff_event_handler );
    lv_obj_t *tilt_label = lv_label_create( tilt_cont, NULL);
    lv_obj_add_style( tilt_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( tilt_label, "tilt to wake");
    lv_obj_align( tilt_label, tilt_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
*/

    lv_obj_t *daily_stepcounter_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(daily_stepcounter_cont, true);
    lv_obj_set_size(daily_stepcounter_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( daily_stepcounter_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( daily_stepcounter_cont, stepcounter_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    daily_stepcounter_onoff = lv_switch_create( daily_stepcounter_cont, NULL );
    lv_obj_add_protect( daily_stepcounter_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( daily_stepcounter_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( daily_stepcounter_onoff, LV_ANIM_ON );
    lv_obj_align( daily_stepcounter_onoff, daily_stepcounter_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( daily_stepcounter_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *daily_stepcounter_label = lv_label_create( daily_stepcounter_cont, NULL);
    lv_obj_add_style( daily_stepcounter_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( daily_stepcounter_label, "not implemented");
    lv_obj_align( daily_stepcounter_label, daily_stepcounter_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *one_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(one_cont, true);
    lv_obj_set_size(one_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( one_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( one_cont, daily_stepcounter_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    one_onoff = lv_switch_create( one_cont, NULL );
    lv_obj_add_protect( one_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( one_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( one_onoff, LV_ANIM_ON );
    lv_obj_align( one_onoff, one_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( one_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *one_label = lv_label_create( one_cont, NULL);
    lv_obj_add_style( one_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( one_label, "one");
    lv_obj_align( one_label, one_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *two_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(one_cont, true);
    lv_obj_set_size(two_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( two_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( two_cont, two_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    two_onoff = lv_switch_create( two_cont, NULL );
    lv_obj_add_protect( two_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( two_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( two_onoff, LV_ANIM_ON );
    lv_obj_align( two_onoff, two_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( two_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *two_label = lv_label_create( two_cont, NULL);
    lv_obj_add_style( two_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( two_label, "two\nlong description");
    lv_obj_align( two_label, two_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *three_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(three_cont, true);
    lv_obj_set_size(three_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( three_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( three_cont, two_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    three_onoff = lv_switch_create( three_cont, NULL );
    lv_obj_add_protect( three_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( three_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( three_onoff, LV_ANIM_ON );
    lv_obj_align( three_onoff, three_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( three_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *three_label = lv_label_create( three_cont, NULL);
    lv_obj_add_style( three_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( three_label, "three");
    lv_obj_align( three_label, three_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *four_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(four_cont, true);
    lv_obj_set_size(four_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( four_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( four_cont, three_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    four_onoff = lv_switch_create( four_cont, NULL );
    lv_obj_add_protect( four_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( four_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( four_onoff, LV_ANIM_ON );
    lv_obj_align( four_onoff, four_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( four_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *four_label = lv_label_create( four_cont, NULL);
    lv_obj_add_style( four_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( four_label, "four");
    lv_obj_align( four_label, four_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *five_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(five_cont, true);
    lv_obj_set_size(five_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( five_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( five_cont, four_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    five_onoff = lv_switch_create( five_cont, NULL );
    lv_obj_add_protect( five_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( five_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( five_onoff, LV_ANIM_ON );
    lv_obj_align( five_onoff, five_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( five_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *five_label = lv_label_create( five_cont, NULL);
    lv_obj_add_style( five_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( five_label, "five");
    lv_obj_align( five_label, five_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *six_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(six_cont, true);
    lv_obj_set_size(six_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( six_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( six_cont, five_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    six_onoff = lv_switch_create( six_cont, NULL );
    lv_obj_add_protect( six_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( six_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( six_onoff, LV_ANIM_ON );
    lv_obj_align( six_onoff, six_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( six_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *six_label = lv_label_create( six_cont, NULL);
    lv_obj_add_style( six_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( six_label, "six");
    lv_obj_align( six_label, six_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *seven_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(seven_cont, true);
    lv_obj_set_size(seven_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( seven_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( seven_cont, six_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    seven_onoff = lv_switch_create( seven_cont, NULL );
    lv_obj_add_protect( seven_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( seven_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( seven_onoff, LV_ANIM_ON );
    lv_obj_align( seven_onoff, seven_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( seven_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *seven_label = lv_label_create( seven_cont, NULL);
    lv_obj_add_style( seven_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( seven_label, "seven");
    lv_obj_align( seven_label, seven_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

    lv_obj_t *eight_cont = lv_obj_create( move_page, NULL );
    lv_page_glue_obj(eight_cont, true);
    lv_obj_set_size(eight_cont, lv_disp_get_hor_res( NULL ) - 10, 50);
    lv_obj_add_style( eight_cont, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_obj_align( eight_cont, seven_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
    eight_onoff = lv_switch_create( eight_cont, NULL );
    lv_obj_add_protect( eight_onoff, LV_PROTECT_CLICK_FOCUS);
    lv_obj_add_style( eight_onoff, LV_SWITCH_PART_INDIC, mainbar_get_switch_style() );
    lv_switch_off( eight_onoff, LV_ANIM_ON );
    lv_obj_align( eight_onoff, eight_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( eight_onoff, daily_stepcounter_onoff_event_handler );
    lv_obj_t *eight_label = lv_label_create( eight_cont, NULL);
    lv_obj_add_style( eight_label, LV_OBJ_PART_MAIN, &move_settings_style  );
    lv_label_set_text( eight_label, "eight");
    lv_obj_align( eight_label, eight_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );

/*
    if ( bma_get_config( BMA_DOUBLECLICK ) )
        lv_switch_on( doubleclick_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( doubleclick_onoff, LV_ANIM_OFF );
*/
    if ( bma_get_config( BMA_STEPCOUNTER ) )
        lv_switch_on( stepcounter_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( stepcounter_onoff, LV_ANIM_OFF );
/*
    if ( bma_get_config( BMA_TILT ) )
        lv_switch_on( tilt_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( tilt_onoff, LV_ANIM_OFF );
*/
    if ( bma_get_config( BMA_DAILY_STEPCOUNTER ) )
        lv_switch_on( daily_stepcounter_onoff, LV_ANIM_OFF );
    else
        lv_switch_off( daily_stepcounter_onoff, LV_ANIM_OFF );
}


static void enter_move_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( move_tile_num, LV_ANIM_OFF );
                                        break;
    }
}

static void exit_move_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( setup_get_tile_num(), LV_ANIM_OFF );
                                        break;
    }
}

static void stepcounter_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  bma_set_config( BMA_STEPCOUNTER, lv_switch_get_state( obj ) );
    }
}
/*
static void doubleclick_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  bma_set_config( BMA_DOUBLECLICK, lv_switch_get_state( obj ) );
    }
}

static void tilt_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  bma_set_config( BMA_TILT, lv_switch_get_state( obj ) );
    }
}
*/
static void daily_stepcounter_onoff_event_handler(lv_obj_t * obj, lv_event_t event) {
    switch( event ) {
        case( LV_EVENT_VALUE_CHANGED):  bma_set_config( BMA_DAILY_STEPCOUNTER, lv_switch_get_state( obj ) );
    }
}