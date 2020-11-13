/****************************************************************************
 *   Copyright  2020  Jesper Ortlund
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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
#include <Arduino.h>
#include "wheelinfo_tile.h"
#include "gui/mainbar/mainbar.h"
#include "hardware/pmu.h"
#include "hardware/wheelctl.h"
#include "hardware/dashboard.h"

uint32_t wheelinfo_tile_num;
lv_task_t *wheelinfo_task = nullptr;

void lv_wheelinfo_task(lv_task_t *wheelinfo_task);
void wheelinfo_setup_styles( void );
void wheelinfo_setup_obj( void );
void wheelinfo_update( void );
void wheelinfo_activate_cb(void);
void wheelinfo_hibernate_cb(void);

static lv_obj_t *wheelinfo_cont = NULL;
static lv_style_t *style;
lv_style_t wheelinfo_style;
lv_style_t wheelinfo_heading_style;
lv_style_t wheelinfo_data_style;

lv_obj_t *voltage_data;
lv_obj_t *current_data;
lv_obj_t *power_data;
lv_obj_t *speed_data;

void wheelinfo_tile_setup(void)
{
    wheelinfo_tile_num = mainbar_add_tile(2, 1, "wheelinfo tile");
    wheelinfo_cont = mainbar_get_tile_obj(wheelinfo_tile_num);
    //fulldash_cont = mainbar_get_tile_obj( mainbar_add_tile( 1, 0, "fulldash tile" ) );
    style = mainbar_get_style();
    Serial.println("setting up wheelinfo_tile");
    wheelinfo_setup_styles();
    wheelinfo_setup_obj();

    mainbar_add_tile_activate_cb(wheelinfo_tile_num, wheelinfo_activate_cb);
    mainbar_add_tile_hibernate_cb(wheelinfo_tile_num, wheelinfo_hibernate_cb);
}

void wheelinfo_setup_styles( void ) {
    lv_style_copy( &wheelinfo_style, mainbar_get_style() );
    lv_style_set_bg_color( &wheelinfo_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
    lv_style_set_bg_opa( &wheelinfo_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &wheelinfo_style, LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( wheelinfo_cont, LV_OBJ_PART_MAIN, &wheelinfo_style );

    lv_style_copy( &wheelinfo_heading_style, &wheelinfo_style );
    lv_style_set_text_color( &wheelinfo_heading_style, LV_OBJ_PART_MAIN, LV_COLOR_WHITE );

    lv_style_copy( &wheelinfo_data_style, &wheelinfo_style );
    lv_style_set_text_color( &wheelinfo_data_style, LV_OBJ_PART_MAIN, LV_COLOR_LIME );
}

void wheelinfo_setup_obj( void ) {

    lv_obj_t *voltage_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( voltage_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( voltage_label, "battery voltage");
    lv_obj_align( voltage_label, wheelinfo_cont, LV_ALIGN_IN_TOP_LEFT, 5, 5 );
    voltage_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( voltage_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( voltage_data, "67 V");
    lv_obj_align( voltage_data, wheelinfo_cont, LV_ALIGN_IN_TOP_RIGHT, -5, 5 );

    lv_obj_t *current_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( current_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( current_label, "current");
    lv_obj_align( current_label, voltage_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    current_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( current_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( current_data, "2.4 A");
    lv_obj_align( current_data, voltage_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );
}

void wheelinfo_activate_cb(void)
{
    wheelinfo_task = lv_task_create(lv_wheelinfo_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(wheelinfo_task);
}

void wheelinfo_hibernate_cb(void)
{
    lv_task_del(wheelinfo_task);
}

void wheelinfo_tile_reload(void)
{
    lv_obj_del(wheelinfo_cont);
    wheelinfo_tile_setup();
}

uint32_t wheelinfo_get_tile(void)
{
    return wheelinfo_tile_num;
}

void lv_wheelinfo_task ( lv_task_t *wheelinfo_task )
{
    wheelinfo_update();
}

void wheelinfo_update( void ) {
    char temp[16]="";
    
    snprintf( temp, sizeof( temp ), "%0.2f V", wheelctl_get_data(WHEELCTL_VOLTAGE) );
    lv_label_set_text( voltage_data, temp);
    lv_obj_align( voltage_data, wheelinfo_cont, LV_ALIGN_IN_TOP_RIGHT, -5, 5 );

    snprintf( temp, sizeof( temp ), "%0.2f A", wheelctl_get_data(WHEELCTL_CURRENT) );
    lv_label_set_text( current_data, temp);
    lv_obj_align( current_data, voltage_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );
}
