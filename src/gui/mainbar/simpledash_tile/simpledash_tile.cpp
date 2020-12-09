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
#include "hardware/blectl.h"
#include "hardware/motor.h"
#include "hardware/wheelctl.h"

//task declarations
lv_task_t *overlay_task = nullptr;

// Function declarations
void simpledash_overlay_task(lv_task_t *overlay_task);
static void sd_overlay_event_cb(lv_obj_t *obj, lv_event_t event);

/*
   Declare LVGL Dashboard objects and styles
*/

LV_IMG_DECLARE(currentalarm_128px);
LV_IMG_DECLARE(battalarm_128px);
LV_IMG_DECLARE(tempalarm_128px);
LV_IMG_DECLARE(fan_40px);

LV_FONT_DECLARE(DIN1451_m_cond_36);
LV_FONT_DECLARE(DIN1451_m_cond_180);

static lv_obj_t *simpledash_cont = NULL;
static lv_style_t *sd_style;
static lv_style_t sd_arc_style;

// Arc gauges and labels
//arc background styles
//Speed
static lv_obj_t *sd_speed_label = nullptr;
static lv_style_t sd_speed_label_style;
//Battery
static lv_obj_t *sd_batt_arc = nullptr;
static lv_style_t sd_batt_indic_style;
static lv_style_t sd_batt_main_style;
//Current
static lv_obj_t *sd_current_arc = nullptr;
static lv_style_t sd_current_indic_style;
static lv_style_t sd_current_main_style;
//Max min avg bars
static lv_obj_t *sd_batt_max_bar = nullptr;
static lv_obj_t *sd_batt_min_bar = nullptr;
static lv_obj_t *sd_current_max_bar = nullptr;
static lv_obj_t *sd_current_regen_bar = nullptr;
//Max avg, regen and min bars
static lv_style_t sd_max_bar_indic_style;
static lv_style_t sd_min_bar_indic_style; //also for avg speed
static lv_style_t sd_regen_bar_indic_style;
static lv_style_t sd_bar_main_style;
//alert objects and styles
static lv_obj_t *sd_batt_alert = NULL;
static lv_obj_t *sd_current_alert = NULL;
static lv_obj_t *sd_temp_alert = NULL;
static lv_obj_t *sd_fan_indic = NULL;
static lv_style_t sd_alert_style;

//Overlay objects and styles
static lv_obj_t *sd_overlay_bar = NULL;
static lv_obj_t *sd_overlay_label = NULL;
static lv_style_t sd_overlay_style;
static lv_style_t sd_overlay_label_style;

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
bool simpledash_active = false;

int sd_out_arc_x = 240;
int sd_out_arc_y = 240;

uint32_t simpledash_tile_num;

/*************************************
 *  Define LVGL default object styles
 ************************************/

void lv_sd_define_styles_1(void)
{
    //style template
    lv_style_copy(&sd_arc_style, sd_style);
    lv_style_set_line_rounded(&sd_arc_style, LV_STATE_DEFAULT, false);
    lv_style_set_line_width(&sd_arc_style, LV_STATE_DEFAULT, sd_arclinew);
    lv_style_set_bg_opa(&sd_arc_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_width(&sd_arc_style, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&sd_arc_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    //General styles

    //Speed label
    lv_style_init(&sd_speed_label_style);
    lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, sd_speed_bg_clr);
    lv_style_set_text_font(&sd_speed_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_180);
    // Battery Arc
    lv_style_copy(&sd_batt_indic_style, &sd_arc_style);
    lv_style_copy(&sd_batt_main_style, &sd_arc_style);
    lv_style_set_line_color(&sd_batt_main_style, LV_STATE_DEFAULT, sd_batt_bg_clr);
    lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, sd_batt_bg_clr);
    // Current Arc
    lv_style_copy(&sd_current_indic_style, &sd_arc_style);
    lv_style_copy(&sd_current_main_style, &sd_arc_style);
    lv_style_set_line_color(&sd_current_main_style, LV_STATE_DEFAULT, sd_current_bg_clr);
    lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, sd_current_bg_clr);
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
    
    //alerts
    lv_style_copy(&sd_alert_style, sd_style);
    lv_style_set_bg_opa(&sd_alert_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    //overlay
    lv_style_copy(&sd_overlay_style, sd_style);
    lv_style_set_bg_color(&sd_overlay_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&sd_overlay_style, LV_STATE_DEFAULT, LV_OPA_30);

    lv_style_copy(&sd_overlay_label_style, &sd_overlay_style);
    lv_style_set_text_color(&sd_overlay_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_font(&sd_overlay_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_36);
    lv_style_set_text_opa(&sd_overlay_label_style, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_bg_opa(&sd_overlay_label_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
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
    lv_label_set_text(sd_speed_label, "0");
    lv_label_set_align(sd_speed_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sd_speed_label, simpledash_cont, LV_ALIGN_CENTER, 0, 0);
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
    lv_arc_set_bg_angles(sd_batt_arc, sd_batt_arc_start, sd_batt_arc_end);
    lv_arc_set_angles(sd_batt_arc, sd_batt_arc_start, sd_batt_arc_end);
    lv_arc_set_range(sd_batt_arc, 0, 100);
    lv_obj_set_size(sd_batt_arc, sd_out_arc_x, sd_out_arc_y);
    lv_obj_align(sd_batt_arc, NULL, LV_ALIGN_CENTER, 0, 0);
    //mainbar_add_slide_element(sd_batt_arc);

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
        //mainbar_add_slide_element(sd_batt_min_bar);
    }
}

void lv_sd_current_arc_1(void)
{
    //Create current gauge arc
    byte maxcurrent = wheelctl_get_constant(WHEELCTL_CONST_MAXCURRENT);
    //Arc
    sd_current_arc = lv_arc_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_current_arc, LV_OBJ_PART_MAIN);
    lv_obj_add_style(sd_current_arc, LV_ARC_PART_INDIC, &sd_current_indic_style);
    lv_obj_add_style(sd_current_arc, LV_OBJ_PART_MAIN, &sd_current_main_style);
    lv_arc_set_type(sd_current_arc, LV_ARC_TYPE_REVERSE);
    lv_arc_set_bg_angles(sd_current_arc, sd_current_arc_start, sd_current_arc_end);
    lv_arc_set_angles(sd_current_arc, sd_current_arc_start, sd_current_arc_end);
    lv_arc_set_range(sd_current_arc, 0, maxcurrent);
    lv_obj_set_size(sd_current_arc, sd_out_arc_x, sd_out_arc_y);
    lv_obj_align(sd_current_arc, NULL, LV_ALIGN_CENTER, 0, 0);
    //mainbar_add_slide_element(sd_current_arc);

    if (dashboard_get_config(DASHBOARD_BARS))
    {
        //Max bar
        sd_current_max_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_current_max_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_current_max_bar, LV_ARC_PART_INDIC, &sd_max_bar_indic_style);
        lv_obj_add_style(sd_current_max_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_current_max_bar, sd_current_arc_start, sd_current_arc_end);
        lv_arc_set_range(sd_current_max_bar, 0, maxcurrent);
        lv_obj_set_size(sd_current_max_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_current_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

        //regen bar
        sd_current_regen_bar = lv_arc_create(simpledash_cont, NULL);
        lv_obj_reset_style_list(sd_current_regen_bar, LV_OBJ_PART_MAIN);
        lv_obj_add_style(sd_current_regen_bar, LV_ARC_PART_INDIC, &sd_regen_bar_indic_style);
        lv_obj_add_style(sd_current_regen_bar, LV_OBJ_PART_MAIN, &sd_bar_main_style);
        lv_arc_set_bg_angles(sd_current_regen_bar, sd_current_arc_start, sd_current_arc_end);
        lv_arc_set_range(sd_current_regen_bar, 0, maxcurrent);
        lv_obj_set_size(sd_current_regen_bar, sd_out_arc_x, sd_out_arc_y);
        lv_obj_align(sd_current_regen_bar, NULL, LV_ALIGN_CENTER, 0, 0);
        //mainbar_add_slide_element(sd_current_regen_bar);
    }
}
void lv_sd_alerts(void) 
{
    sd_fan_indic = lv_img_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_fan_indic, LV_OBJ_PART_MAIN);
    lv_obj_set_size(sd_fan_indic, 32, 32);
    lv_obj_add_style(sd_fan_indic, LV_OBJ_PART_MAIN, &sd_alert_style);
    lv_img_set_src(sd_fan_indic, &fan_40px);
    lv_obj_set_hidden(sd_fan_indic, true);
    lv_obj_align(sd_fan_indic, NULL, LV_ALIGN_IN_TOP_RIGHT, -5, 5);

    sd_batt_alert = lv_img_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_batt_alert, LV_OBJ_PART_MAIN);
    lv_obj_set_size(sd_batt_alert, 128, 128);
    lv_obj_add_style(sd_batt_alert, LV_OBJ_PART_MAIN, &sd_alert_style);
    lv_img_set_src(sd_batt_alert, &battalarm_128px);
    lv_obj_set_hidden(sd_batt_alert, true);
    lv_obj_align(sd_batt_alert, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

    sd_current_alert = lv_img_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_current_alert, LV_OBJ_PART_MAIN);
    lv_obj_set_size(sd_current_alert, 128, 128);
    lv_obj_add_style(sd_current_alert, LV_OBJ_PART_MAIN, &sd_alert_style);
    lv_img_set_src(sd_current_alert, &currentalarm_128px);
    lv_obj_set_hidden(sd_current_alert, true);
    lv_obj_align(sd_current_alert, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    sd_temp_alert = lv_img_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_temp_alert, LV_OBJ_PART_MAIN);
    lv_obj_set_size(sd_temp_alert, 128, 128);
    lv_obj_add_style(sd_temp_alert, LV_OBJ_PART_MAIN, &sd_alert_style);
    lv_img_set_src(sd_temp_alert, &tempalarm_128px);
    lv_obj_set_hidden(sd_temp_alert, true);
    lv_obj_align(sd_temp_alert, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
}

void lv_sd_overlay(void)
{
    
    sd_overlay_bar = lv_bar_create(simpledash_cont, NULL);
    lv_obj_reset_style_list(sd_overlay_bar, LV_OBJ_PART_MAIN);
    lv_obj_set_size(sd_overlay_bar, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_add_style(sd_overlay_bar, LV_OBJ_PART_MAIN, &sd_overlay_style);
    lv_obj_align(sd_overlay_bar, NULL, LV_ALIGN_CENTER, 0, 0);
    mainbar_add_slide_element(sd_overlay_bar);
    lv_obj_set_event_cb(sd_overlay_bar, sd_overlay_event_cb);

    sd_overlay_label = lv_label_create(sd_overlay_bar, NULL);
    lv_label_set_text(sd_overlay_label, "disconnected");
    lv_obj_reset_style_list(sd_overlay_label, LV_OBJ_PART_MAIN);
    lv_obj_add_style(sd_overlay_label, LV_OBJ_PART_MAIN, &sd_overlay_label_style);
    lv_obj_align(sd_overlay_label, NULL, LV_ALIGN_CENTER, 0, 0);
    mainbar_add_slide_element(sd_overlay_label);
    lv_obj_set_event_cb(sd_overlay_label, sd_overlay_event_cb);

} //End Create Dashboard objects

static void sd_overlay_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event) {
        case (LV_EVENT_LONG_PRESSED):
            log_e("long press on overlay");
            motor_vibe(5, true);
            wheelctl_toggle_lights();
    }
}

void simpledash_current_alert(bool enabled) {
    lv_obj_set_hidden(sd_current_alert , !enabled);
}
void simpledash_batt_alert(bool enabled) {
    lv_obj_set_hidden(sd_batt_alert , !enabled);
}
void simpledash_temp_alert(bool enabled) {
    lv_obj_set_hidden(sd_temp_alert , !enabled);
}
void simpledash_fan_indic(bool enabled) {
    lv_obj_set_hidden(sd_fan_indic , !enabled);
}

int sd_value2angle(int arcstart, int arcstop, float minvalue, float maxvalue, float arcvalue, bool reverse)
{
    int rAngle;
    int arcdegrees;

    if (arcstop < arcstart) arcdegrees = (arcstop + 360) - arcstart;
    else arcdegrees = arcstop - arcstart;
    
    if (reverse) rAngle = arcstop - (arcvalue * arcdegrees / (maxvalue - minvalue));
    else rAngle = arcstart + (arcvalue * arcdegrees / (maxvalue - minvalue));

    if (rAngle >= 360) rAngle = rAngle - 360;
    else if (rAngle < 0) rAngle = rAngle + 360;

    return rAngle;
}

void simpledash_speed_update(float current_speed, float warn_speed, float tiltback_speed, float top_speed)
{
    if (sd_speed_label == NULL) return;
    if (current_speed >= tiltback_speed)
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    }
    else if (current_speed >= warn_speed)
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    }
    else
    {
        lv_style_set_text_color(&sd_speed_label_style, LV_STATE_DEFAULT, sd_speed_fg_clr);
    }
    lv_obj_add_style(sd_speed_label, LV_LABEL_PART_MAIN, &sd_speed_label_style);

    if (dashboard_get_config(DASHBOARD_IMPDIST)) current_speed = current_speed / 1.6;
    
    static char sd_speedstring[4];
    dtostrf(current_speed, 2, 0, sd_speedstring);
    
    lv_label_set_text(sd_speed_label, sd_speedstring);
    lv_label_set_align(sd_speed_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sd_speed_label, simpledash_cont, LV_ALIGN_CENTER, 0, 0);
}

void simpledash_batt_update(float current_battpct, float min_battpct, float max_battpct)
{
    if (sd_batt_arc == NULL) return;
    if (current_battpct < wheelctl_get_constant(WHEELCTL_CONST_BATTCRIT))
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    }
    else if (current_battpct < wheelctl_get_constant(WHEELCTL_CONST_BATTWARN))
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    }
    else
    {
        lv_style_set_line_color(&sd_batt_indic_style, LV_STATE_DEFAULT, sd_batt_fg_clr);
    }
    lv_obj_add_style(sd_batt_arc, LV_ARC_PART_INDIC, &sd_batt_indic_style);

    // update batt arc

    lv_arc_set_value(sd_batt_arc, current_battpct);

    if (dashboard_get_config(DASHBOARD_BARS))
    {
        if (sd_batt_max_bar == NULL || sd_batt_min_bar == NULL) return;
        int ang_max = sd_value2angle(sd_batt_arc_start, sd_batt_arc_end, 0, 100, max_battpct, sd_rev_batt_arc);
        int ang_max2 = ang_max + 3;
        if (ang_max2 >= 360)
        {
            ang_max2 = ang_max2 - 360;
        }
        lv_arc_set_angles(sd_batt_max_bar, ang_max, ang_max2);

        int ang_min = sd_value2angle(sd_batt_arc_start, sd_batt_arc_end, 0, 100, min_battpct, sd_rev_batt_arc);
        int ang_min2 = ang_min + 3;
        if (ang_min2 >= 360)
        {
            ang_min2 = ang_min2 - 360;
        }
        lv_arc_set_angles(sd_batt_min_bar, ang_min, ang_min2);
    }
}

void simpledash_current_update(float current_current, byte maxcurrent, float min_current, float max_current)
{
     
    if (dashboard_get_config(DASHBOARD_CURRENT))
    {
        if (sd_current_arc == NULL) return;
        // Set warning and alert colour
        float amps = current_current;

        if (current_current > (maxcurrent * 0.75))
        {
            lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
        }
        else if (current_current > (maxcurrent * 0.5))
        {
            lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
        }
        else if (current_current < 0)
        {
            lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, sd_speed_fg_clr);
            amps = (current_current * -1);
        }
        else
        {
            lv_style_set_line_color(&sd_current_indic_style, LV_STATE_DEFAULT, sd_current_fg_clr);
        }
        lv_obj_add_style(sd_current_arc, LV_ARC_PART_INDIC, &sd_current_indic_style);
        lv_arc_set_value(sd_current_arc, (maxcurrent - amps));
        
        if (dashboard_get_config(DASHBOARD_BARS))
        {
            if (sd_current_max_bar == NULL || sd_current_regen_bar == NULL) return;
            int ang_max = sd_value2angle(sd_current_arc_start, sd_current_arc_end, 0, maxcurrent, max_current, true);
            int ang_max2 = ang_max + 3;
            if (ang_max2 >= 360)
            {
                ang_max2 = ang_max2 - 360;
            }
            lv_arc_set_angles(sd_current_max_bar, ang_max, ang_max2);

            int ang_regen = sd_value2angle(sd_current_arc_start, sd_current_arc_end, 0, maxcurrent, min_current, true);
            int ang_regen2 = ang_regen + 3;
            if (ang_regen2 >= 360)
            {
                ang_regen2 = ang_regen2 - 360;
            }
            lv_arc_set_angles(sd_current_regen_bar, ang_regen, ang_regen2);
        }
    }
}

void simpledash_overlay_update()
{
    if (sd_overlay_bar == NULL || sd_overlay_label == NULL) return;
    if (blectl_cli_getconnected())
    {
        lv_style_set_bg_opa(&sd_overlay_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_add_style(sd_overlay_bar, LV_OBJ_PART_MAIN, &sd_overlay_style);
        lv_obj_set_hidden(sd_overlay_label, true);
    }
    else
    {
        lv_style_set_bg_opa(&sd_overlay_style, LV_STATE_DEFAULT, LV_OPA_30);
        lv_obj_add_style(sd_overlay_bar, LV_OBJ_PART_MAIN, &sd_overlay_style);
        lv_obj_set_hidden(sd_overlay_label, false);
    }
}

void simpledash_overlay_task(lv_task_t *overlay_task)
{
    simpledash_overlay_update();
}

uint32_t simpledash_get_tile(void)
{
    return simpledash_tile_num;
}

void simpledash_activate_cb(void)
{
    if (!simpledash_active) {
        overlay_task = lv_task_create(simpledash_overlay_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
        lv_task_ready(overlay_task);
        simpledash_active = true;
        wheelctl_update_values();
    }
}

void simpledash_hibernate_cb(void)
{
    if (simpledash_active && overlay_task != nullptr) {
        lv_task_del(overlay_task);
        simpledash_active = false;
    }
}

void simpledash_tile_reload(void)
{
    lv_obj_del(simpledash_cont);
    simpledash_tile_setup();
}

void simpledash_tile_setup(void)
{
    simpledash_tile_num = mainbar_add_tile(2, 0, "sd tile");
    simpledash_cont = mainbar_get_tile_obj(simpledash_tile_num);
    sd_style = mainbar_get_style();
    log_i("setting up dashboard");
    lv_sd_define_styles_1();
    lv_sd_alerts();
    lv_sd_speed_arc_1();
    lv_sd_batt_arc_1();
    if (dashboard_get_config(DASHBOARD_CURRENT)) lv_sd_current_arc_1();
    lv_sd_overlay();

    mainbar_add_tile_activate_cb(simpledash_tile_num, simpledash_activate_cb);
    mainbar_add_tile_hibernate_cb(simpledash_tile_num, simpledash_hibernate_cb);
}