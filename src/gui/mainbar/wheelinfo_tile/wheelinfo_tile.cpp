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
//static lv_style_t *style;
lv_style_t wheelinfo_style;
lv_style_t wheelinfo_heading_style;
lv_style_t wheelinfo_data_style;

lv_obj_t *voltage_data;
lv_obj_t *current_data;
lv_obj_t *colour_data;
lv_obj_t *speed_data;
lv_obj_t *maxvolt_data;
lv_obj_t *model_data;
lv_obj_t *serial_data;
lv_obj_t *capacity_data;
lv_obj_t *odometer_data;

void wheelinfo_tile_setup(void)
{
    wheelinfo_tile_num = mainbar_add_tile(2, 1, "wheelinfo tile");
    wheelinfo_cont = mainbar_get_tile_obj(wheelinfo_tile_num);
    //style = mainbar_get_style();
    log_i("setting up wheelinfo_tile");
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

    lv_obj_t *heading_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( heading_label, LV_OBJ_PART_MAIN, &wheelinfo_heading_style );
    lv_label_set_text( heading_label, "wheel information");
    lv_obj_align( heading_label, wheelinfo_cont, LV_ALIGN_IN_TOP_MID, 0, 5 );

    lv_obj_t *serial_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( serial_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( serial_label, "serial");
    lv_obj_align( serial_label, wheelinfo_cont, LV_ALIGN_IN_TOP_LEFT, 5, 30 );
    serial_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( serial_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( serial_data, "disconnected");
    lv_obj_align( serial_data, wheelinfo_cont, LV_ALIGN_IN_TOP_RIGHT, -5, 30 );

    lv_obj_t *model_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( model_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( model_label, "model");
    lv_obj_align( model_label, serial_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    model_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( model_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( model_data, "n/a");
    lv_obj_align( model_data, serial_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_obj_t *odometer_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( odometer_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( odometer_label, "odometer");
    lv_obj_align( odometer_label, model_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    odometer_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( odometer_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( odometer_data, "n/a");
    lv_obj_align( odometer_data, model_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_obj_t *maxvolt_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( maxvolt_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( maxvolt_label, "max voltage");
    lv_obj_align( maxvolt_label, odometer_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    maxvolt_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( maxvolt_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( maxvolt_data, "n/a");
    lv_obj_align( maxvolt_data, odometer_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_obj_t *voltage_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( voltage_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( voltage_label, "current voltage");
    lv_obj_align( voltage_label, maxvolt_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    voltage_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( voltage_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( voltage_data, "n/a");
    lv_obj_align( voltage_data, maxvolt_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_obj_t *capacity_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( capacity_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( capacity_label, "battery capacity");
    lv_obj_align( capacity_label, voltage_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    capacity_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( capacity_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( capacity_data, "n/a");
    lv_obj_align( capacity_data, voltage_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_obj_t *colour_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( colour_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( colour_label, "colour");
    lv_obj_align( colour_label, capacity_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    colour_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( colour_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( colour_data, "n/a");
    lv_obj_align( colour_data, capacity_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

/*
    lv_obj_t *blesvcdata_label = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( blesvcdata_label, LV_OBJ_PART_MAIN, &wheelinfo_style );
    lv_label_set_text( blesvcdata_label, "blesvcdata");
    lv_obj_align( blesvcdata_label, colour_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0 );
    blesvcdata_data = lv_label_create( wheelinfo_cont, NULL);
    lv_obj_add_style( blesvcdata_data, LV_OBJ_PART_MAIN, &wheelinfo_data_style  );
    lv_label_set_text( blesvcdata_data, "2.4 A");
    lv_obj_align( blesvcdata_data, colour_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );
    */
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
    lv_obj_clean(wheelinfo_cont);
    wheelinfo_setup_obj();
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
    char temp[26]="no data";

    lv_label_set_text( serial_data, wheelctl_get_info(WHEELCTL_INFO_SERIAL).c_str());
    lv_obj_align( serial_data, wheelinfo_cont, LV_ALIGN_IN_TOP_RIGHT, -5, 30 );

    lv_label_set_text( model_data, wheelctl_get_info(WHEELCTL_INFO_MODEL).c_str());
    lv_obj_align( model_data, serial_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    if (dashboard_get_config(DASHBOARD_IMPDIST)) {
        float impodo = wheelctl_get_data(WHEELCTL_ODO) / 1.6;
        snprintf( temp, sizeof( temp ), "%0.1f mi", impodo );
    } else {
        snprintf( temp, sizeof( temp ), "%0.1f km", wheelctl_get_data(WHEELCTL_ODO) );
    }
    lv_label_set_text( odometer_data, temp);
    lv_obj_align( odometer_data, model_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    snprintf( temp, sizeof( temp ), "%d V", wheelctl_get_constant(WHEELCTL_CONST_BATTVOLT) );
    lv_label_set_text( maxvolt_data, temp);
    lv_obj_align( maxvolt_data, odometer_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    snprintf( temp, sizeof( temp ), "%0.2f V", wheelctl_get_data(WHEELCTL_VOLTAGE) );
    lv_label_set_text( voltage_data, temp);
    lv_obj_align( voltage_data, maxvolt_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_label_set_text( capacity_data, wheelctl_get_info(WHEELCTL_INFO_BATTCAP).c_str());
    lv_obj_align( capacity_data, voltage_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    lv_label_set_text( colour_data, wheelctl_get_info(WHEELCTL_INFO_WHEELCOLOR).c_str());
    lv_obj_align( colour_data, capacity_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );
/*
    //snprintf( temp, sizeof( temp ), "%s", wheelctl_get_info(WHEELCTL_INFO_BLESUUID) );
    strcpy(temp, wheelctl_get_info(WHEELCTL_INFO_BLESUUID).c_str());
    
    //snprintf( temp, sizeof( temp ), "%s", wheelctl_get_info(WHEELCTL_INFO_BLESDUUID) );
    strcpy(temp, wheelctl_get_info(WHEELCTL_INFO_BLESDUUID).c_str());
    lv_label_set_text( blesduuid_data, temp);
    lv_obj_align( blesduuid_data, blesuuid_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    //snprintf( temp, sizeof( temp ), "%s", wheelctl_get_info(WHEELCTL_INFO_BLEDATA) );
    strcpy(temp, wheelctl_get_info(WHEELCTL_INFO_BLEDATA).c_str());
    lv_label_set_text( blesvcdata_data, temp);
    lv_obj_align( blesvcdata_data, blesduuid_data, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0 );

    //Serial.println(wheelctl_get_info(WHEELCTL_INFO_maxvolt));
    */
}
