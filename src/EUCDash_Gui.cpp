#include "twatch_config.h"
#include <Arduino.h>
#include <time.h>
#include "EUCDash.h"
#include "string.h"
#include <Ticker.h>
#include "lv_gui.h"

/***********************************************
   Mike, Change these to false if you want the simple dash
 ***********************************************/
bool fulldash = true; // display the full or the simple dash version?
bool dcurrentarc = true; //display current arc on the simple dash?
//bool fulldash = false;
//bool dcurrentarc = false;
/**************End Dash settings****************/

bool rev_current_arc = false; //default direction of current arc
bool rev_batt_arc = true; //default direction of battery arc
int arclinew;
bool shakeoff[3] = {true, true, true};
extern float wheeldata[];
//extern struct Wheel_constants wheelconst;

/******************************
   LVGL Gui code
 ******************************/

/*
  typedef struct {
  lv_obj_t *bgarc;
  lv_style_t *bgarc_style;
  lv_style_t *fgarc_style;
  } arc_obj_t;

  arc_obj_t *speedarc;

  void lv_draw_arc(int arcbegin, int arcend, int minvalue, float maxvalue, int sizex, int sizey, lv_style_t bg_style, lv_style_t fg_style, lv_obj_t* object) {
  object = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(object, LV_ARC_PART_INDIC, &bg_style);
  lv_obj_add_style(object, LV_OBJ_PART_MAIN, &fg_style);
  lv_arc_set_bg_angles(object, arcbegin, arcend);
  lv_arc_set_range(object, minvalue, maxvalue);
  lv_obj_set_size(object, sizex, sizey);
  lv_obj_align(object, NULL, LV_ALIGN_CENTER, 0, 0);
  }
*/

void lv_setup_dashstyle() {
  if (fulldash) {
    arclinew = 15; // line width of arc gauges
    speed_arc_start = 160;
    speed_arc_end = 20;
    batt_arc_start = 35;
    batt_arc_end = 145;
    current_arc_start = 130;
    current_arc_end = 230;
    temp_arc_start = 310;
    temp_arc_end = 50;
    rev_batt_arc = true;
    rev_current_arc = false;
  } else if (dcurrentarc) {
    arclinew = 25;
    batt_arc_start = 110;
    batt_arc_end = 250;
    current_arc_start = 290;
    current_arc_end = 70;
    rev_batt_arc = false;
    rev_current_arc = true;
  } else {
    arclinew = 25;
    batt_arc_start = 20;
    batt_arc_end = 340;
    rev_batt_arc = false;
  }
}



/************************************
   Define LVGL default object styles
 ************************************/
void lv_define_styles_1(void) {

  //General styles
  lv_style_init(&arc_warn_style);
  lv_style_set_line_rounded(&arc_warn_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&arc_warn_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&arc_warn_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&arc_warn_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_opa(&arc_warn_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  //Speed arc and label

  lv_style_init(&speed_indic_style);
  lv_style_set_line_rounded(&speed_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_indic_style, LV_STATE_DEFAULT, arclinew);

  lv_style_init(&speed_main_style);
  lv_style_set_line_rounded(&speed_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_color(&speed_main_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_line_color(&speed_main_style, LV_STATE_DEFAULT, speed_bg_clr);
  if (!fulldash) {
    lv_style_set_line_opa(&speed_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  }
  lv_style_init(&speed_label_style);
  lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, speed_fg_clr);
  if (fulldash) {
    lv_style_set_text_font(&speed_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_120);
  } else {
    lv_style_set_text_font(&speed_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_180);
  }

  // Battery Arc and label
  lv_style_init(&batt_indic_style);
  lv_style_set_line_rounded(&batt_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&batt_indic_style, LV_STATE_DEFAULT, arclinew);

  lv_style_init(&batt_main_style);
  lv_style_set_line_rounded(&batt_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&batt_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&batt_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_color(&batt_main_style, LV_STATE_DEFAULT, batt_bg_clr);

  lv_style_init(&batt_label_style);
  lv_style_set_text_font(&batt_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_66);

  // Current Arc and label
  lv_style_init(&current_indic_style);
  lv_style_set_line_rounded(&current_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&current_indic_style, LV_STATE_DEFAULT, arclinew);

  lv_style_init(&current_main_style);
  lv_style_set_line_rounded(&current_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&current_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&current_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&current_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_color(&current_main_style, LV_STATE_DEFAULT, current_bg_clr);

  lv_style_init(&current_label_style);
  lv_style_set_text_font(&current_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_44);

  // Temperature Arc
  lv_style_init(&temp_indic_style);
  lv_style_set_line_rounded(&temp_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&temp_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, temp_fg_clr);

  lv_style_init(&temp_main_style);
  lv_style_set_line_rounded(&temp_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&temp_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&temp_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&temp_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_color(&temp_main_style, LV_STATE_DEFAULT, temp_bg_clr);

  lv_style_init(&temp_label_style);
  lv_style_set_text_font(&temp_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_44);


  //Bar background -- transparent
  lv_style_init(&bar_main_style);
  lv_style_set_line_rounded(&bar_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&bar_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_opa(&bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  //Max bar
  lv_style_init(&max_bar_indic_style);
  lv_style_set_line_rounded(&max_bar_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&max_bar_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&max_bar_indic_style, LV_STATE_DEFAULT, max_bar_clr);

  //min bar
  lv_style_init(&min_bar_indic_style);
  lv_style_set_line_rounded(&min_bar_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&min_bar_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&min_bar_indic_style, LV_STATE_DEFAULT, min_bar_clr);

  //regen bar
  lv_style_init(&regen_bar_indic_style);
  lv_style_set_line_rounded(&regen_bar_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&regen_bar_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&regen_bar_indic_style, LV_STATE_DEFAULT, regen_bar_clr);

  // Clock
  lv_style_init(&dashtime_style);
  lv_style_set_text_color(&dashtime_style, LV_STATE_DEFAULT, watch_info_colour);
  lv_style_set_text_font(&dashtime_style, LV_STATE_DEFAULT, &DIN1451_m_cond_28);

  //Trip meter
  lv_style_init(&trip_label_style);
  lv_style_set_text_color(&trip_label_style, LV_STATE_DEFAULT, current_fg_clr);
  lv_style_set_text_font(&trip_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_44);

} //End Define LVGL default object styles

/***************************
    Create Dashboard objects
 ***************************/
void lv_speed_arc_1(void)
{
  //  if (speed_main_style != nullptr) {
  //   lv_draw_arc(160, 20, 0, (wheeldata[15] + 5), 268, 268, speedarc->bgarc_style, speedarc->fgarc_style, speedarc->bgarc);
  // }
  // lv_draw_arc(160, 20, 0, (wheeldata[15] + 5), 268, 268, speed_main_style, speed_indic_style, speed_arc);
  /*Create speed gauge arc*/
  //Arc

  speed_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(speed_arc, LV_ARC_PART_INDIC, &speed_indic_style);
  lv_obj_add_style(speed_arc, LV_OBJ_PART_MAIN, &speed_main_style);
  lv_arc_set_bg_angles(speed_arc, speed_arc_start, speed_arc_end);
  lv_arc_set_range(speed_arc, 0, (wheeldata[15] + 5));
  lv_arc_set_value(speed_arc, wheeldata[1]);
  lv_obj_set_size(speed_arc, 268, 268);
  lv_obj_align(speed_arc, NULL, LV_ALIGN_CENTER, 0, 0);


  if (fulldash) {
    //Max bar
    speed_max_bar = lv_arc_create(lv_scr_act(), NULL);
    lv_obj_add_style(speed_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
    lv_obj_add_style(speed_max_bar, LV_OBJ_PART_MAIN, &bar_main_style);
    lv_arc_set_bg_angles(speed_max_bar, speed_arc_start, speed_arc_end);
    lv_arc_set_range(speed_max_bar, 0, (wheeldata[15] + 5));
    lv_obj_set_size(speed_max_bar, 268, 268);
    lv_obj_align(speed_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

    //avg bar
    speed_avg_bar = lv_arc_create(lv_scr_act(), NULL);
    lv_obj_add_style(speed_avg_bar, LV_ARC_PART_INDIC, &min_bar_indic_style);
    lv_obj_add_style(speed_avg_bar, LV_OBJ_PART_MAIN, &bar_main_style);
    lv_arc_set_bg_angles(speed_avg_bar, speed_arc_start, speed_arc_end);
    lv_arc_set_range(speed_avg_bar, 0, (wheeldata[15] + 5));
    lv_obj_set_size(speed_avg_bar, 268, 268);
    lv_obj_align(speed_avg_bar, NULL, LV_ALIGN_CENTER, 0, 0);
  }

  //Label
  speed_label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(speed_label, LV_LABEL_PART_MAIN, &speed_label_style);
  char speedstring[4];
  if (wheeldata[1] > 10) {
    dtostrf(wheeldata[1], 2, 0, speedstring);
  } else {
    dtostrf(wheeldata[1], 1, 0, speedstring);
  }
  lv_label_set_text(speed_label, speedstring);
  lv_label_set_align(speed_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(speed_label, speed_arc, LV_ALIGN_CENTER, 0, -5);
}

void lv_batt_arc_1(void)
{
  /*Create battery gauge arc*/
  //Arc
  batt_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_arc, LV_ARC_PART_INDIC, &batt_indic_style);
  lv_obj_add_style(batt_arc, LV_OBJ_PART_MAIN, &batt_main_style);
  if (rev_batt_arc) {
    lv_arc_set_type(batt_arc, LV_ARC_TYPE_REVERSE);
  }
  lv_arc_set_bg_angles(batt_arc, batt_arc_start, batt_arc_end);
  lv_arc_set_angles(batt_arc, batt_arc_start, batt_arc_end);
  lv_arc_set_range(batt_arc, 0, 100);
  lv_obj_set_size(batt_arc, 268, 268);
  lv_obj_align(batt_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //max bar
  batt_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(batt_max_bar, LV_OBJ_PART_MAIN, &bar_main_style);
  lv_arc_set_bg_angles(batt_max_bar, batt_arc_start, batt_arc_end);
  lv_arc_set_range(batt_max_bar, 0, 100);
  lv_obj_set_size(batt_max_bar, 268, 268);
  lv_obj_align(batt_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //min bar
  batt_min_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_min_bar, LV_ARC_PART_INDIC, &min_bar_indic_style);
  lv_obj_add_style(batt_min_bar, LV_OBJ_PART_MAIN, &bar_main_style);
  lv_arc_set_bg_angles(batt_min_bar, batt_arc_start, batt_arc_end);
  lv_arc_set_range(batt_min_bar, 0, 100);
  lv_obj_set_size(batt_min_bar, 268, 268);
  lv_obj_align(batt_min_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //Label
  if (fulldash) {
    batt_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(batt_label, LV_OBJ_PART_MAIN, &batt_label_style);
    char battstring[4];
    dtostrf(wheeldata[6], 2, 0, battstring);
    lv_label_set_text(batt_label, battstring);
    lv_label_set_align(batt_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(batt_label, batt_arc, LV_ALIGN_CENTER, 0, 75);
  }
}

void lv_current_arc_1(void)
{
  /*Create current gauge arc*/
  int arcw = 225;
  int arch = 225;

  //Arc
  current_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_arc, LV_ARC_PART_INDIC, &current_indic_style);
  lv_obj_add_style(current_arc, LV_OBJ_PART_MAIN, &current_main_style);
  if (rev_current_arc) {
    lv_arc_set_type(current_arc, LV_ARC_TYPE_REVERSE);
    arcw = 268;
    arch = 268;
  }
  lv_arc_set_bg_angles(current_arc, current_arc_start, current_arc_end);
  lv_arc_set_angles(current_arc, current_arc_start, current_arc_end);
  lv_arc_set_range(current_arc, 0, wheelconst.maxcurrent);
  lv_obj_set_size(current_arc, arcw, arch);
  lv_obj_align(current_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //Max bar
  current_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(current_max_bar, LV_OBJ_PART_MAIN, &bar_main_style);
  lv_arc_set_bg_angles(current_max_bar, current_arc_start, current_arc_end);
  lv_arc_set_range(current_max_bar, 0, wheelconst.maxcurrent);
  lv_obj_set_size(current_max_bar, arcw, arch);
  lv_obj_align(current_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //regen bar
  current_regen_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_regen_bar, LV_ARC_PART_INDIC, &regen_bar_indic_style);
  lv_obj_add_style(current_regen_bar, LV_OBJ_PART_MAIN, &bar_main_style);
  lv_arc_set_bg_angles(current_regen_bar, current_arc_start, current_arc_end);
  lv_arc_set_range(current_regen_bar, 0, wheelconst.maxcurrent);
  lv_obj_set_size(current_regen_bar, arcw, arch);
  lv_obj_align(current_regen_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //Label
  if (fulldash) {
    current_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(current_label, LV_OBJ_PART_MAIN, &current_label_style);
    char currentstring[4];
    dtostrf(wheeldata[3], 2, 0, currentstring);
    lv_label_set_text(current_label, currentstring);
    lv_label_set_align(current_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(current_label, current_arc, LV_ALIGN_CENTER, -64, 0);
  }
}

void lv_temp_arc_1(void)
{
  /*Create temprature gauge arc*/
  //Arc
  temp_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_arc, LV_ARC_PART_INDIC, &temp_indic_style);
  lv_obj_add_style(temp_arc, LV_OBJ_PART_MAIN, &temp_main_style);
  lv_arc_set_type(temp_arc, LV_ARC_TYPE_REVERSE);
  lv_arc_set_bg_angles(temp_arc, temp_arc_start, temp_arc_end);
  lv_arc_set_angles(temp_arc, temp_arc_start, temp_arc_end);
  lv_arc_set_range(temp_arc, 0, (wheelconst.crittemp + 10));
  lv_arc_set_value(temp_arc, ((wheelconst.crittemp + 10) - wheeldata[4]));
  lv_obj_set_size(temp_arc, 225, 225);
  lv_obj_align(temp_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //Max bar
  temp_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(temp_max_bar, LV_OBJ_PART_MAIN, &bar_main_style);
  lv_arc_set_bg_angles(temp_max_bar, temp_arc_start, temp_arc_end);
  lv_arc_set_range(temp_max_bar, 0, (wheelconst.crittemp + 10));
  lv_obj_set_size(temp_max_bar, 225, 225);
  lv_obj_align(temp_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //Label
  temp_label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_label, LV_OBJ_PART_MAIN, &temp_label_style);
  char tempstring[4];
  dtostrf(wheeldata[4], 2, 0, tempstring);
  lv_label_set_text(temp_label, tempstring);
  lv_label_set_align(temp_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(temp_label, temp_arc, LV_ALIGN_CENTER, 64, 0);
}

void lv_dashtime(void) {
  dashtime = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(dashtime, LV_OBJ_PART_MAIN, &dashtime_style);
  lv_obj_align(dashtime, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  wbatt = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(wbatt, LV_OBJ_PART_MAIN, &dashtime_style);
  lv_obj_align(wbatt, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -25);
  trip = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(trip, LV_OBJ_PART_MAIN, &trip_label_style);
  lv_obj_align(trip, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);
} //End Create Dashboard objects

int value2angle(int arcstart, int arcstop, float minvalue, float maxvalue, float arcvalue, bool reverse) {
  int rAngle;
  int arcdegrees;
  if (arcstop < arcstart) {
    arcdegrees = (arcstop + 360) - arcstart;
  } else {
    arcdegrees = arcstop - arcstart;
  }
  if (reverse) {
    rAngle = arcstop - (arcvalue * arcdegrees / (maxvalue - minvalue));
  } else {
    rAngle = arcstart + (arcvalue * arcdegrees / (maxvalue - minvalue));
  }
  if (rAngle >= 360) {
    rAngle = rAngle - 360;
  } else if (rAngle < 0) {
    rAngle = rAngle + 360;
  }
  return rAngle;
}

/***************************************************************
   Dashboard GUI Update Functions, called via the task handler
   runs every 250ms
 ***************************************************************/
static void lv_speed_update(void) {
  if (wheeldata[1] >= wheeldata[15]) {
    lv_style_set_line_color(&speed_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[1] >= wheeldata[14]) {
    lv_style_set_line_color(&speed_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&speed_indic_style, LV_STATE_DEFAULT, speed_fg_clr);
    lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, speed_fg_clr);
  }
  if (fulldash) {

    lv_obj_add_style(speed_arc, LV_ARC_PART_INDIC, &speed_indic_style);
    lv_arc_set_value(speed_arc, wheeldata[1]);

    int ang_max = value2angle(speed_arc_start, speed_arc_end, 0, (wheeldata[15] + 5), max_speed, false);
    int ang_max2 = ang_max + 3;
    if (ang_max2 >= 360) {
      ang_max2 = ang_max2 - 360;
    }
    lv_arc_set_angles(speed_max_bar, ang_max, ang_max2);

    int ang_avg = value2angle(speed_arc_start, speed_arc_end, 0, (wheeldata[15] + 5), avg_speed, false);
    int ang_avg2 = ang_avg + 3;
    if (ang_avg2 >= 360) {
      ang_avg2 = ang_avg2 - 360;
    }
    lv_arc_set_angles(speed_avg_bar, ang_avg, ang_avg2);
  }
  lv_obj_add_style(speed_label, LV_LABEL_PART_MAIN, &speed_label_style);
  char speedstring[4];
  if (wheeldata[1] > 10) {
    dtostrf(wheeldata[1], 2, 0, speedstring);
  } else {
    dtostrf(wheeldata[1], 1, 0, speedstring);
  }
  lv_label_set_text(speed_label, speedstring);
  lv_label_set_align(speed_label, LV_LABEL_ALIGN_CENTER);
  if (fulldash) {
    lv_obj_align(speed_label, speed_arc, LV_ALIGN_CENTER, 0, -3);
  } else {
    lv_obj_align(speed_label, speed_arc, LV_ALIGN_CENTER, 0, 8);
  }
}

void lv_batt_update(void) {
  if (wheeldata[6] < 10) {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[6] < wheelconst.battwarn) {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, batt_fg_clr);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, batt_fg_clr);
  }
  lv_obj_add_style(batt_arc, LV_ARC_PART_INDIC, &batt_indic_style);
  if (rev_batt_arc) {
    lv_arc_set_value(batt_arc, (100 - wheeldata[6]));
  } else {
    lv_arc_set_value(batt_arc, wheeldata[6]);
  }
  int ang_max = value2angle(batt_arc_start, batt_arc_end, 0, 100, max_batt, rev_batt_arc);
  int ang_max2 = ang_max + 3;
  if (ang_max2 >= 360) {
    ang_max2 = ang_max2 - 360;
  }
  lv_arc_set_angles(batt_max_bar, ang_max, ang_max2);

  int ang_min = value2angle(batt_arc_start, batt_arc_end, 0, 100, min_batt, rev_batt_arc);
  int ang_min2 = ang_min + 3;
  if (ang_min2 >= 360) {
    ang_min2 = ang_min2 - 360;
  }
  lv_arc_set_angles(batt_min_bar, ang_min, ang_min2);

  //lv_arc_set_angles(batt_max_bar, (142 - (max_batt * 110 / 100)), (144 - (max_batt * 110 / 100)));
  //lv_arc_set_angles(batt_min_bar, (142 - (min_batt * 110 / 100)), (144 - (min_batt * 110 / 100)));

  if (fulldash) {
    lv_obj_add_style(batt_label, LV_OBJ_PART_MAIN, &batt_label_style);
    char battstring[4];
    if (wheeldata[6] > 10) {
      dtostrf(wheeldata[6], 2, 0, battstring);
    } else {
      dtostrf(wheeldata[6], 1, 0, battstring);
    }
    lv_label_set_text(batt_label, battstring);
    lv_label_set_align(batt_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(batt_label, batt_arc, LV_ALIGN_CENTER, 0, 75);
  }
}

void lv_current_update(void) {
  // Set warning and alert colour
  float amps = wheeldata[3];
  if (wheeldata[3] > (wheelconst.maxcurrent * 0.75)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[3] > (wheelconst.maxcurrent * 0.5)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else if (wheeldata[3] < 0) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, speed_fg_clr);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, speed_fg_clr);
    amps = (wheeldata[3] * -1);
  } else {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, current_fg_clr);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, current_fg_clr);
    //stop_current_shake();
  }
  lv_obj_add_style(current_arc, LV_ARC_PART_INDIC, &current_indic_style);

  if (rev_current_arc) {
    lv_arc_set_value(current_arc, (wheelconst.maxcurrent - amps));
    //lv_arc_set_value(current_arc, 10);
  } else {
    lv_arc_set_value(current_arc, amps);
  }

  int ang_max = value2angle(current_arc_start, current_arc_end, 0, wheelconst.maxcurrent, max_current, rev_current_arc);
  int ang_max2 = ang_max + 3;
  if (ang_max2 >= 360) {
    ang_max2 = ang_max2 - 360;
  }
  lv_arc_set_angles(current_max_bar, ang_max, ang_max2);

  int ang_regen = value2angle(current_arc_start, current_arc_end, 0, wheelconst.maxcurrent, regen_current, rev_current_arc);
  int ang_regen2 = ang_regen + 3;
  if (ang_regen2 >= 360) {
    ang_regen2 = ang_regen2 - 360;
  }
  lv_arc_set_angles(current_regen_bar, ang_regen, ang_regen2);

  // lv_arc_set_angles(current_max_bar, (130 + (max_current * 100 / wheelconst.maxcurrent)), (133 + (max_current * 100 / wheelconst.maxcurrent)));
  if (fulldash) {
    lv_obj_add_style(current_label, LV_OBJ_PART_MAIN, &current_label_style);
    char currentstring[4];
    dtostrf(wheeldata[3], 2, 0, currentstring);
    lv_label_set_text(current_label, currentstring);
    lv_label_set_align(current_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(current_label, NULL, LV_ALIGN_CENTER, -64, 0);
  }
}

void lv_temp_update(void) {
  // Set warning and alert colour
  if (wheeldata[4] > wheelconst.crittemp) {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[4] > wheelconst.warntemp) {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, temp_fg_clr);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, temp_fg_clr);
  }
  lv_obj_add_style(temp_arc, LV_ARC_PART_INDIC, &temp_indic_style);
  lv_arc_set_value(temp_arc, ((wheelconst.crittemp + 10) - wheeldata[4]));

  int ang_max = value2angle(temp_arc_start, temp_arc_end, 0, (wheelconst.crittemp + 10), max_temp, true);
  int ang_max2 = ang_max + 3;
  if (ang_max2 >= 360) {
    ang_max2 = ang_max2 - 360;
  }
  lv_arc_set_angles(temp_max_bar, ang_max, ang_max2);

  lv_obj_add_style(temp_label, LV_OBJ_PART_MAIN, &temp_label_style);
  char tempstring[4];
  dtostrf(wheeldata[4], 2, 0, tempstring);
  lv_label_set_text(temp_label, tempstring);
  lv_label_set_align(temp_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(temp_label, NULL, LV_ALIGN_CENTER, 64, 0);
}

//Update function for the clock display when wheel is disconnected
void updateTime()
{
  TTGOClass *ttgo = TTGOClass::getWatch();
  time_t now;
  struct tm  info;
  char buf[64];
  time(&now);
  localtime_r(&now, &info);
  strftime(buf, sizeof(buf), "%H:%M", &info);
  int watchbatt = ttgo->power->getBattPercentage();
  if (connected) {
    if (dashtime != nullptr) {
      lv_label_set_text(dashtime, buf);
      lv_obj_align(dashtime, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    }
    if (wbatt != nullptr) {
      char wbattstring[4];
      dtostrf(watchbatt, 2, 0, wbattstring);
      lv_label_set_text(wbatt, wbattstring);
      lv_obj_align(wbatt, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -25);
    }
    if (trip != nullptr) {
      char tripstring[6];
      dtostrf(wheeldata[8], 2, 1, tripstring);
      lv_label_set_text(trip, tripstring);
      lv_obj_align(trip, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);
    }
  } else {
    if (timeLabel != nullptr) {
      lv_label_set_text(timeLabel, buf);
      lv_obj_align(timeLabel, NULL, LV_ALIGN_CENTER, 0, -20);
    }
    if (dateLabel != nullptr) {
      strftime(buf, sizeof(buf), "%a %h %d %Y", &info);
      lv_label_set_text (dateLabel, buf);
      lv_obj_align(dateLabel, NULL, LV_ALIGN_CENTER, 0, 47);
    }
    char watchbattstring[4];
    char wbs[6] = {0};
    char pctst[2] = "%";
    dtostrf(watchbatt, 2, 0, watchbattstring);
    snprintf(wbs, sizeof(wbs), "%s%s", watchbattstring, pctst);
    if (battLabel != nullptr) {
      lv_label_set_text (battLabel, wbs);
      lv_label_set_align(battLabel, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(battLabel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
  }
  ttgo->rtc->syncToRtc();
}

/*
   sets the backlight level according to the time of day
   adjust this to your local conditions,
   would be nice to have a functione to set this
   automatically according to latitude and day of year
   todo: add a separate task that runs less often for this
*/

void update_speed_shake(void) {
  if (wheeldata[1] >= wheeldata[14] && shakeoff[0]) {
    speed_shake = lv_task_create(lv_speed_shake, 750, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(speed_shake);
    shakeoff[0] = false;
  } else if (!shakeoff[0]) {
    stop_speed_shake();
  }
}

void update_current_shake(void) {
  if (wheeldata[3] > (wheelconst.maxcurrent * 0.75) && shakeoff[1]) {
    current_shake = lv_task_create(lv_current_shake, 200, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(current_shake);
    shakeoff[1] = false;
  } else if (!shakeoff[1]) {
    stop_current_shake();
  }
}

void update_temp_shake(void) {
  if (wheeldata[4] > wheelconst.crittemp && shakeoff[2]) {
    temp_shake = lv_task_create(lv_temp_shake, 1000, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(temp_shake);
    shakeoff[2] = false;
  } else if (!shakeoff[2]) {
    stop_temp_shake();
  }
}

/************************
   Task update functions
 ***********************/
static void lv_dash_task(lv_task_t * dash_task) {
  if (!displayOff) {
    lv_speed_update();
    lv_batt_update();
    if (fulldash) {
      lv_current_update();
      lv_temp_update();
    }
    else if (dcurrentarc) {
      lv_current_update();
    }
  }
  update_speed_shake();
  update_current_shake();
  update_temp_shake();
}

static void lv_time_task(lv_task_t * time_task) {
  updateTime();
}

static void lv_current_shake(lv_task_t * current_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
}
static void lv_temp_shake(lv_task_t * temp_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
  delay(250);
  ttgo->shake ();
}

static void lv_speed_shake(lv_task_t * speed_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
}

void lv_set_clock_bg(void) {
  lv_style_init(&clock_bg_style);
  lv_style_set_radius(&clock_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_style_set_bg_color(&clock_bg_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
  lv_style_set_bg_opa(&clock_bg_style, LV_OBJ_PART_MAIN, LV_OPA_COVER);
  lv_style_set_border_width(&clock_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_obj_t *clock_bg = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(clock_bg, 240, 240);
  lv_obj_add_style(clock_bg, LV_OBJ_PART_MAIN, &clock_bg_style);
}

void lv_set_dash_bg(void) {
  lv_style_init(&dash_bg_style);
  lv_style_set_radius(&dash_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_style_set_bg_color(&dash_bg_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
  lv_style_set_bg_opa(&dash_bg_style, LV_OBJ_PART_MAIN, LV_OPA_COVER);
  lv_style_set_border_width(&dash_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_obj_t *dash_bg = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(dash_bg, 240, 240);
  lv_obj_add_style(dash_bg, LV_OBJ_PART_MAIN, &dash_bg_style);
}

/******************************************************
   Setup functions declared in the .h file and can be called
   from the .ino file
 ***************************************************/

void setup_timeGui(void) {
  Serial.println("setting up clock");
  lv_set_clock_bg();
  lv_style_init(&timeLabel_style);
  lv_style_set_text_font(&timeLabel_style, LV_STATE_DEFAULT, &DIN1451_m_cond_120);
  lv_style_set_text_color(&timeLabel_style, LV_STATE_DEFAULT, watch_colour);
  timeLabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(timeLabel, LV_OBJ_PART_MAIN, &timeLabel_style);
  lv_label_set_align(timeLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(timeLabel, temp_arc, LV_ALIGN_CENTER, 0, -20);

  lv_style_init(&dateLabel_style);
  lv_style_set_text_font(&dateLabel_style, LV_STATE_DEFAULT, &DIN1451_m_cond_36);
  lv_style_set_text_color(&dateLabel_style, LV_STATE_DEFAULT, watch_colour);

  dateLabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(dateLabel, LV_OBJ_PART_MAIN, &dateLabel_style);
  lv_label_set_align(dateLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(dateLabel, temp_arc, LV_ALIGN_CENTER, 0, 47);

  lv_style_init(&batt_fg_style);
  lv_style_set_text_font(&batt_fg_style, LV_STATE_DEFAULT, &DIN1451_m_cond_28);
  lv_style_set_text_color(&batt_fg_style, LV_STATE_DEFAULT, watch_bg_colour);

  battLabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(battLabel, LV_OBJ_PART_MAIN, &batt_fg_style);
  lv_label_set_align(battLabel, LV_LABEL_ALIGN_RIGHT);
  lv_obj_align(battLabel, temp_arc, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  //run time update task
  time_task = lv_task_create(lv_time_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
  lv_task_ready(time_task);
}

void setup_LVGui(void) {
  Serial.println("setting up dashboard");
  lv_set_dash_bg();
  lv_setup_dashstyle();
  lv_define_styles_1();
  lv_speed_arc_1();
  lv_batt_arc_1();
  if (fulldash) {
    lv_current_arc_1();
    lv_temp_arc_1();
    lv_dashtime();
    time_task = lv_task_create(lv_time_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
    lv_task_ready(time_task);
  }
  else if (dcurrentarc) {
    lv_current_arc_1();
  }
  //Create task -- update freq 4/s
  dash_task = lv_task_create(lv_dash_task, 250, LV_TASK_PRIO_LOWEST, NULL);
  lv_task_ready(dash_task);
}


void stop_time_task() {
  Serial.println("check if clock is running");
  if (time_task != nullptr) {
    Serial.println("shutting down clock");
    lv_task_del(time_task);
  }
}

void stop_dash_task() {
  Serial.println("check if dash is running");
  if (dash_task != nullptr) {
    Serial.println("shutting down dash");
    lv_task_del(dash_task);
  }
  if (time_task != nullptr) {
    Serial.println("shutting down dashclock");
    lv_task_del(time_task);
  }
}

void stop_speed_shake() {
  if (speed_shake != nullptr) {
    lv_task_del(speed_shake);
    shakeoff[0] = true;
  }
}

void stop_current_shake() {
  if (current_shake != nullptr) {
    lv_task_del(current_shake);
    shakeoff[1] = true;
  }
}

void stop_temp_shake() {
  if (temp_shake != nullptr) {
    lv_task_del(temp_shake);
    shakeoff[2] = true;
  }
}

/*******************
   End LVGL GUI Code
 *******************/
