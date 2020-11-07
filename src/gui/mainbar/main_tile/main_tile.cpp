/****************************************************************************
 *   modified 2020 Jesper Ortlund
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
#include <stdio.h>
#include <time.h>

#include "config.h"
#include "main_tile.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/setup_tile/time_settings/time_settings.h"

#include "hardware/timesync.h"
#include "hardware/powermgm.h"
#include "hardware/pmu.h"
#include "hardware/alloc.h"

static lv_obj_t *main_cont = NULL;
static lv_obj_t *clock_cont = NULL;
static lv_obj_t *timelabel = NULL;
static lv_obj_t *datelabel = NULL;
static lv_obj_t *battlabel = NULL;
uint32_t main_tile_num;

static lv_style_t *style;
static lv_style_t timestyle;
static lv_style_t datestyle;
static lv_style_t battstyle;

LV_FONT_DECLARE(Ubuntu_72px);
LV_FONT_DECLARE(Ubuntu_16px);
LV_FONT_DECLARE(DIN1451_m_cond_28);
LV_FONT_DECLARE(DIN1451_m_cond_36);
LV_FONT_DECLARE(DIN1451_m_cond_120);

static lv_color_t watch_colour = lv_color_make(0xFF, 0x00, 0x00);    // (Red)

lv_task_t * main_tile_task;

void main_tile_update_task( lv_task_t * task );
void main_tile_align_widgets( void );
void main_tile_format_time( char *, size_t, struct tm * );
bool main_tile_powermgm_event_cb( EventBits_t event, void *arg );

void main_tile_setup( void ) {
    main_tile_num = mainbar_add_tile( 0, 0, "main tile" );
    main_cont = mainbar_get_tile_obj( main_tile_num );
    style = mainbar_get_style();

    lv_style_copy( &timestyle, style);
    lv_style_set_text_color( &timestyle, LV_OBJ_PART_MAIN, watch_colour );
    lv_style_set_text_font( &timestyle, LV_STATE_DEFAULT, &DIN1451_m_cond_120);

    lv_style_copy( &datestyle, style);
    lv_style_set_text_color( &datestyle, LV_OBJ_PART_MAIN, watch_colour );
    lv_style_set_text_font( &datestyle, LV_STATE_DEFAULT, &DIN1451_m_cond_36);

    lv_style_copy( &battstyle, style);
    lv_style_set_text_color( &battstyle, LV_OBJ_PART_MAIN, watch_colour );
    lv_style_set_text_font( &battstyle, LV_STATE_DEFAULT, &DIN1451_m_cond_28);

    clock_cont = mainbar_obj_create( main_cont );
    lv_obj_set_size( clock_cont, lv_disp_get_hor_res( NULL ) , lv_disp_get_ver_res( NULL ));
    lv_obj_add_style( clock_cont, LV_OBJ_PART_MAIN, style );
    lv_obj_align( clock_cont, main_cont, LV_ALIGN_CENTER, 0, 0 );

    timelabel = lv_label_create( clock_cont , NULL);
    lv_label_set_text(timelabel, "00:00");
    lv_obj_reset_style_list( timelabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( timelabel, LV_OBJ_PART_MAIN, &timestyle );
    lv_obj_align(timelabel, clock_cont, LV_ALIGN_CENTER, 0, -20);

    datelabel = lv_label_create( clock_cont , NULL);
    lv_label_set_text(datelabel, "1 Jan 1970");
    lv_obj_reset_style_list( datelabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( datelabel, LV_OBJ_PART_MAIN, &datestyle );
    lv_obj_align( datelabel, clock_cont, LV_ALIGN_CENTER, 0, 47 );

    battlabel = lv_label_create( clock_cont , NULL);
    lv_label_set_text(battlabel, "99");
    lv_obj_reset_style_list( battlabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( battlabel, LV_OBJ_PART_MAIN, &battstyle );
    lv_obj_align( battlabel, clock_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -5 );

    struct tm  info;
    char buf[64];

    main_tile_format_time( buf, sizeof(buf), &info );
    lv_label_set_text( timelabel, buf );
    strftime( buf, sizeof(buf), "%a %d %b %Y", &info );
    lv_label_set_text( datelabel, buf );
    lv_obj_align( datelabel, timelabel, LV_ALIGN_CENTER, 0, 47 );

    main_tile_task = lv_task_create( main_tile_update_task, 500, LV_TASK_PRIO_MID, NULL );

    powermgm_register_cb( POWERMGM_WAKEUP , main_tile_powermgm_event_cb, "main tile time update" );
}

bool main_tile_powermgm_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case POWERMGM_WAKEUP:
            main_tile_update_time();
            break;
    }
    return( true );
}

uint32_t main_tile_get_tile_num( void ) {
    return( main_tile_num );
}

void main_tile_update_time( void ) {
    struct tm  info;
    char time_str[64]="";
    static char *old_time_str = NULL;

    // on first run, alloc psram
    if ( old_time_str == NULL ) {
        old_time_str = (char *)CALLOC( sizeof( time_str), 1 );
        if ( old_time_str == NULL ) {
            log_e("old_time_str allocation failed");
            while(true);
        }
    }

    main_tile_format_time( time_str, sizeof(time_str), &info );

    // only update while time_str changes
    if ( strcmp( time_str, old_time_str ) ) {
        log_i("renew time_str: %s != %s", time_str, old_time_str );
        lv_label_set_text( timelabel, time_str );
        lv_obj_align( timelabel, clock_cont, LV_ALIGN_CENTER, 0, -20 );
        strlcpy( old_time_str, time_str, sizeof( time_str ) );

        strftime( time_str, sizeof(time_str), "%a %d %b %Y", &info );
        lv_label_set_text( datelabel, time_str );
        lv_obj_align( datelabel, clock_cont, LV_ALIGN_CENTER, 0, 47 );
    }    
}

void main_tile_update_batt( void ) {
    int32_t percent = pmu_get_battery_percent();
    uint8_t level = (uint8_t)percent;
    char pctst[2] = "%";
    char battstring[6] = {0};
    char levelst[2] = {0};
    if (level > 100)
        level = 100;
    dtostrf(level, 2, 0, levelst);
    snprintf(battstring, sizeof(battstring), "%s%s", levelst, pctst);
    lv_label_set_text(battlabel, battstring);
    lv_obj_align( battlabel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5 );
}

void main_tile_update_task( lv_task_t * task ) {
    main_tile_update_time();
    main_tile_update_batt();
}

void main_tile_format_time( char * buf, size_t buf_len, struct tm * info ) {
    time_t now;

    time( &now );
    localtime_r( &now, info );
    int h = info->tm_hour;
    int m = info->tm_min;

    if ( timesync_get_24hr() ) {
        snprintf( buf, buf_len, "%02d:%02d", h, m );
    }
    else {
        if (h == 0) h = 12;
        if (h > 12) h -= 12;
        snprintf( buf, buf_len, "%d:%02d", h, m );
    }
}
