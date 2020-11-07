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
#include "gui/mainbar/mainbar.h"
#include "simpledash_tile.h"
#include "hardware/pmu.h"
#include "hardware/Kingsong.h"
#include "hardware/dashboard.h"

//task declarations
lv_task_t *sd_dash_task = nullptr;

// Function declarations
static void lv_sd_dash_task(lv_task_t *sd_dash_task);

void sd_stop_dash_task();

/*
   Declare LVGL Dashboard objects and styles
*/

static lv_obj_t *simpledash_cont = NULL;
static lv_style_t *style;
static lv_style_t sd_arc_style;

// Arc gauges and labels
//arc background styles
static lv_style_t sd_arc_warn_style;
static lv_style_t sd_arc_crit_style;
//Speed
static lv_obj_t *sd_speed_arc = nullptr;
static lv_obj_t *sd_speed_warn_arc = nullptr;
static lv_obj_t *sd_speed_crit_arc = nullptr;
static lv_obj_t *sd_speed_label = nullptr;
static lv_style_t sd_speed_indic_style;
static lv_style_t sd_speed_main_style;
static lv_style_t sd_speed_label_style;
//Battery
static lv_obj_t *sd_batt_arc = nullptr;
static lv_obj_t *sd_batt_warn_arc = nullptr;
static lv_obj_t *sd_batt_crit_arc = nullptr;
static lv_obj_t *sd_batt_label = nullptr;
static lv_style_t sd_batt_indic_style;
static lv_style_t sd_batt_main_style;
static lv_style_t sd_batt_label_style;
//Current
static lv_obj_t *sd_current_arc = nullptr;
static lv_obj_t *sd_current_warn_arc = nullptr;
static lv_obj_t *sd_current_crit_arc = nullptr;
static lv_obj_t *sd_current_label = nullptr;
static lv_style_t sd_current_indic_style;
static lv_style_t sd_current_main_style;
static lv_style_t sd_current_label_style;

//Max min avg bars
static lv_obj_t *sd_speed_avg_bar = nullptr;
static lv_obj_t *sd_speed_max_bar = nullptr;
static lv_obj_t *sd_batt_max_bar = nullptr;
static lv_obj_t *sd_batt_min_bar = nullptr;
static lv_obj_t *sd_current_max_bar = nullptr;
static lv_obj_t *sd_current_regen_bar = nullptr;

//Max avg, regen and min bars
static lv_style_t sd_max_bar_indic_style;
static lv_style_t sd_min_bar_indic_style; //also for avg speed
static lv_style_t sd_regen_bar_indic_style;
static lv_style_t sd_bar_main_style;

//End LV objects and styles

bool sd_display_current = false;

int sd_batt_arc_start = 65;
int sd_batt_arc_end = 295;
bool sd_rev_batt_arc = false;
int sd_current_arc_start = 290;
int sd_current_arc_end = 70;
bool sd_rev_current_arc = true;
int sd_arclinew = 25; // line width of arc gauges
bool sd_display_bars = true;

int sd_out_arc_x = 240;
int sd_out_arc_y = 240;

extern float wheeldata[];
uint32_t simpledash_tile_num;

/*************************************
 *  Define LVGL default object styles
 ************************************/

void lv_sd_define_styles_1(void)
{
    //style template
    lv_style_copy(&sd_arc_style, style);
    //lv_style_init(&arc_style);
    lv_style_set_line_rounded(&sd_arc_style, LV_STATE_DEFAULT, false);
    lv_style_set_line_width(&sd_arc_style, LV_STATE_DEFAULT, sd_arclinew);
    lv_style_set_bg_opa(&sd_arc_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_width(&sd_arc_style, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&sd_arc_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    //General styles
    lv_style_copy(&sd_arc_warn_style, &sd_arc_style);
    lv_style_set_line_opa(&sd_arc_warn_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    //Speed label
    lv_style_init(&sd_speed_label_style);
    lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, sd_speed_fg_clr);
    lv_style_set_text_font(&sd_speed_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_180);

    // Battery Arc
    lv_style_copy(&sd_batt_indic_style, &sd_arc_style);
    lv_style_copy(&sd_batt_main_style, &sd_arc_style);
    lv_style_set_line_color(&sd_batt_main_style, LV_STATE_DEFAULT, sd_batt_bg_clr);

    // Current Arc
    lv_style_copy(&sd_current_indic_style, &sd_arc_style);
    lv_style_copy(&sd_current_main_style, &sd_arc_style);
    lv_style_set_line_color(&sd_current_main_style, LV_STATE_DEFAULT, sd_current_bg_clr);

    //Bar background -- transparent
    lv_style_copy(&sd_bar_main_style, &sd_arc_style);
    lv_style_set_line_opa(&sd_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    //Max bar
    lv_style_copy(&sd_max_bar_indic_style, &sd_arc_style);
    lv_style_set_line_color(&sd_max_bar_indic_style, LV_STATE_DEFAULT, sd_max_bar_clr);

    //min bar
    lv_style_copy(&sd_min_bar_indic_style, &sd_arc_style);
    lv_style_set_line_color(&sd_min_bar_indic_style, LV_STATE_DEFAULT, sd_min_bar_clr);

    //regen bar
    lv_style_copy(&sd_regen_bar_indic_style, &sd_arc_style);
    lv_style_set_line_color(&sd_regen_bar_indic_style, LV_STATE_DEFAULT, sd_regen_bar_clr);

} //End Define LVGL default object styles

/***************************
    Create Dashboard objects
 ***************************/

void lv_sd_speed_arc_1(void)
{
    //Label
    sd_speed_label = lv_label_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_speed_label, LV_OBJ_PART_MAIN);
    lv_obj_add_style(sd_speed_label, LV_LABEL_PART_MAIN, &sd_speed_label_style);
    char speedstring[4];
    if (wheeldata[1] > 10)
    {
        dtostrf(wheeldata[1], 2, 0, speedstring);
    }
    else
    {
        dtostrf(wheeldata[1], 1, 0, speedstring);
    }
    lv_label_set_text(sd_speed_label, speedstring);
    lv_label_set_align(sd_speed_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sd_speed_label, sd_speed_arc, LV_ALIGN_CENTER, 0, 8);
    mainbar_add_slide_element(sd_speed_label);
}

void lv_sd_batt_arc_1(void)
{
    /*Create battery gauge arc*/

    if (dashboard_get_config(DASHBOARD_CURRENT))
    {
        sd_batt_arc_start = 110;
        sd_batt_arc_end = 250;
    }
    else
    {
        sd_batt_arc_start = 65;
        sd_batt_arc_end = 295;
    }

    //Arc
    sd_batt_arc = lv_arc_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_batt_arc, LV_OBJ_PART_MAIN);
    lv_obj_add_style(sd_batt_arc, LV_ARC_PART_INDIC, &sd_batt_indic_style);
    lv_obj_add_style(sd_batt_arc, LV_OBJ_PART_MAIN, &sd_batt_main_style);
    //lv_arc_set_type(sd_batt_arc, LV_ARC_TYPE_REVERSE);
    lv_arc_set_bg_angles(sd_batt_arc, sd_batt_arc_start, sd_batt_arc_end);
    lv_arc_set_angles(sd_batt_arc, sd_batt_arc_start, sd_batt_arc_end);
    lv_arc_set_range(sd_batt_arc, 0, 100);
    lv_obj_set_size(sd_batt_arc, sd_out_arc_x, sd_out_arc_y);
    lv_obj_align(sd_batt_arc, NULL, LV_ALIGN_CENTER, 0, 0);
    mainbar_add_slide_element(sd_batt_arc);

    if (dashboard_get_config(DASHBOARD_BARS))
    {
        //max bar
        sd_batt_max_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_batt_max_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_batt_max_bar, LV_ARC_PART_INDIC, &sd_max_bar_indic_style);
        lv_obj_add_style(sd_batt_max_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_batt_max_bar, sd_batt_arc_start, sd_batt_arc_end);
        lv_arc_set_range(sd_batt_max_bar, 0, 100);
        lv_obj_set_size(sd_batt_max_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_batt_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

        //min bar
        sd_batt_min_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_batt_min_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_batt_min_bar, LV_ARC_PART_INDIC, &sd_min_bar_indic_style);
        lv_obj_add_style(sd_batt_min_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_batt_min_bar, sd_batt_arc_start, sd_batt_arc_end);
        lv_arc_set_range(sd_batt_min_bar, 0, 100);
        lv_obj_set_size(sd_batt_min_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_batt_min_bar, NULL, LV_ALIGN_CENTER, 0, 0);
        mainbar_add_slide_element(sd_batt_min_bar);
    }
}

void lv_sd_current_arc_1(void)
{
    //Create current gauge arc

    int arcw = 225;
    int arch = 225;

    //Arc
    sd_current_arc = lv_arc_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_current_arc, LV_OBJ_PART_MAIN);
    lv_obj_add_style(sd_current_arc, LV_ARC_PART_INDIC, &sd_current_indic_style);
    lv_obj_add_style(sd_current_arc, LV_OBJ_PART_MAIN, &sd_current_main_style);
    lv_arc_set_bg_angles(sd_current_arc, sd_current_arc_start, sd_current_arc_end);
    lv_arc_set_angles(sd_current_arc, sd_current_arc_start, sd_current_arc_end);
    lv_arc_set_range(sd_current_arc, 0, wheelconst.maxcurrent);
    lv_obj_set_size(sd_current_arc, sd_out_arc_x, sd_out_arc_y);
    lv_obj_align(sd_current_arc, NULL, LV_ALIGN_CENTER, 0, 0);
    mainbar_add_slide_element(sd_current_arc);
    if (dashboard_get_config(DASHBOARD_BARS))
    {
        //Max bar
        sd_current_max_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_current_max_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_current_max_bar, LV_ARC_PART_INDIC, &sd_max_bar_indic_style);
        lv_obj_add_style(sd_current_max_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_current_max_bar, sd_current_arc_start, sd_current_arc_end);
        lv_arc_set_range(sd_current_max_bar, 0, wheelconst.maxcurrent);
        lv_obj_set_size(sd_current_max_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_current_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

        //regen bar
        sd_current_regen_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_current_regen_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_current_regen_bar, LV_ARC_PART_INDIC, &sd_regen_bar_indic_style);
        lv_obj_add_style(sd_current_regen_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_current_regen_bar, sd_current_arc_start, sd_current_arc_end);
        lv_arc_set_range(sd_current_regen_bar, 0, wheelconst.maxcurrent);
        lv_obj_set_size(sd_current_regen_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_current_regen_bar, NULL, LV_ALIGN_CENTER, 0, 0);
        mainbar_add_slide_element(sd_current_regen_bar);
    }
}

int sd_value2angle(int arcstart, int arcstop, float minvalue, float maxvalue, float arcvalue, bool reverse)
{
    int rAngle;
    int arcdegrees;
    if (arcstop < arcstart)
    {
        arcdegrees = (arcstop + 360) - arcstart;
    }
    else
    {
        arcdegrees = arcstop - arcstart;
    }
    if (reverse)
    {
        rAngle = arcstop - (arcvalue * arcdegrees / (maxvalue - minvalue));
    }
    else
    {
        rAngle = arcstart + (arcvalue * arcdegrees / (maxvalue - minvalue));
    }
    if (rAngle >= 360)
    {
        rAngle = rAngle - 360;
    }
    else if (rAngle < 0)
    {
        rAngle = rAngle + 360;
    }
    return rAngle;
}

/***************************************************************
   Dashboard GUI Update Functions, called via the task handler
   runs every 250ms
 ***************************************************************/

static void lv_sd_speed_update(void)
{
    if (wheeldata[1] >= wheeldata[15])
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    }
    else if (wheeldata[1] >= wheeldata[14])
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    }
    else
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, sd_speed_fg_clr);
    }

    lv_obj_add_style(sd_speed_label, LV_LABEL_PART_MAIN, &sd_speed_label_style);
    char speedstring[4];
    float converted_speed = wheeldata[1];
    if (dashboard_get_config(DASHBOARD_IMPDIST)) {
        converted_speed = wheeldata[1] / 1.6;
    }
    if (wheeldata[1] > 10)
    {
        dtostrf(wheeldata[1], 2, 0, speedstring);
    }
    else
    {
        dtostrf(wheeldata[1], 1, 0, speedstring);
    }
    lv_label_set_text(sd_speed_label, speedstring);
    lv_label_set_align(sd_speed_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sd_speed_label, sd_speed_arc, LV_ALIGN_CENTER, 0, 8);
}

void lv_sd_batt_update(void)
{
    if (wheeldata[6] < 10)
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    }
    else if (wheeldata[6] < wheelconst.battwarn)
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    }
    else
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, sd_batt_fg_clr);
    }
    lv_obj_add_style(sd_batt_arc, LV_ARC_PART_INDIC, &sd_batt_indic_style);

    // draw batt arc

    lv_arc_set_value(sd_batt_arc, wheeldata[6]);

    if (dashboard_get_config(DASHBOARD_BARS))
    {
        int ang_max = sd_value2angle(sd_batt_arc_start, sd_batt_arc_end, 0, 100, max_batt, sd_rev_batt_arc);
        int ang_max2 = ang_max + 3;
        if (ang_max2 >= 360)
        {
            ang_max2 = ang_max2 - 360;
        }
        lv_arc_set_angles(sd_batt_max_bar, ang_max, ang_max2);

        int ang_min = sd_value2angle(sd_batt_arc_start, sd_batt_arc_end, 0, 100, min_batt, sd_rev_batt_arc);
        int ang_min2 = ang_min + 3;
        if (ang_min2 >= 360)
        {
            ang_min2 = ang_min2 - 360;
        }
        lv_arc_set_angles(sd_batt_min_bar, ang_min, ang_min2);
    }
}

void lv_sd_current_update(void)
{
    // Set warning and alert colour
    float amps = wheeldata[3];
    if (wheeldata[3] > (wheelconst.maxcurrent * 0.75))
    {
        lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    }
    else if (wheeldata[3] > (wheelconst.maxcurrent * 0.5))
    {
        lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    }
    else if (wheeldata[3] < 0)
    {
        lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, sd_speed_fg_clr);
        amps = (wheeldata[3] * -1);
    }
    else
    {
        lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, sd_current_fg_clr);
    }
    lv_obj_add_style(sd_current_arc, LV_ARC_PART_INDIC, &sd_current_indic_style);

    if (sd_rev_current_arc)
    {
        lv_arc_set_value(sd_current_arc, (wheelconst.maxcurrent - amps));
    }
    else
    {
        lv_arc_set_value(sd_current_arc, amps);
    }
    if (dashboard_get_config(DASHBOARD_BARS))
    {
        int ang_max = sd_value2angle(sd_current_arc_start, sd_current_arc_end, 0, wheelconst.maxcurrent, max_current, sd_rev_current_arc);
        int ang_max2 = ang_max + 3;
        if (ang_max2 >= 360)
        {
            ang_max2 = ang_max2 - 360;
        }
        lv_arc_set_angles(sd_current_max_bar, ang_max, ang_max2);

        int ang_regen = sd_value2angle(sd_current_arc_start, sd_current_arc_end, 0, wheelconst.maxcurrent, regen_current, sd_rev_current_arc);
        int ang_regen2 = ang_regen + 3;
        if (ang_regen2 >= 360)
        {
            ang_regen2 = ang_regen2 - 360;
        }
        lv_arc_set_angles(sd_current_regen_bar, ang_regen, ang_regen2);
    }
}

/************************
   Task update functions
 ***********************/

static void lv_sd_dash_task(lv_task_t *sd_dash_task)
{
    lv_sd_speed_update();
    lv_sd_batt_update();
    if (dashboard_get_config(DASHBOARD_CURRENT))
    {
        lv_sd_current_update();
    }
}

void sd_stop_dash_task()
{
    Serial.println("check if dash is running");
    if (sd_dash_task != nullptr)
    {
        Serial.println("shutting down dash");
        lv_task_del(sd_dash_task);
    }
}

uint32_t simpledash_get_tile(void)
{
    return simpledash_tile_num;
}

void simpledash_tile_reload ( void ) {
    lv_obj_del(simpledash_cont);
    simpledash_tile_setup();
}

void simpledash_tile_setup(void)
{
    simpledash_tile_num = mainbar_add_tile(2, 0, "sd tile");
    simpledash_cont = mainbar_get_tile_obj(simpledash_tile_num);
    //simpledash_cont = mainbar_get_tile_obj(mainbar_add_tile(2, 0, "simpledash tile"));
    style = mainbar_get_style();

    Serial.println("setting up dashboard");

    lv_sd_define_styles_1();
    lv_sd_speed_arc_1();
    lv_sd_batt_arc_1();
    if (dashboard_get_config(DASHBOARD_CURRENT))
    {
        lv_sd_current_arc_1();
    }
    //Create task -- update freq 4/s
    sd_dash_task = lv_task_create(lv_sd_dash_task, 250, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(sd_dash_task);
}