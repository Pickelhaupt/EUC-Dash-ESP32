#include "twatch_config.h"
#include <Arduino.h>
#include <time.h>
#include "EUCDash_test.h"
#include "string.h"
#include <Ticker.h>
#include "BLEDevice.h"

/***********************************************
   Mike, Change these to false if you want the simple dash
 ***********************************************/
bool fulldash = true;
bool display_speedarc = true;

extern float wheeldata[];

lv_task_t *dash_task = nullptr;
lv_task_t *time_task = nullptr;
lv_task_t *speed_shake = nullptr;
//lv_task_t *current_shake = nullptr;
//lv_task_t *temp_shake = nullptr;

// Function declarations
static void lv_dash_task(lv_task_t *dash_task);
static void lv_time_task(lv_task_t *time_task);
static void lv_speed_shake(lv_task_t *speed_shake);
//static void lv_current_shake(lv_task_t *current_shake);
//static void lv_temp_shake(lv_task_t *temp_shake);

/*
   Declare LVGL Dashboard objects and styles
*/

// static void updateTime();

// Arc gauges and labels
static lv_obj_t *speed_arc = nullptr;
static lv_obj_t *speed_label = nullptr;
static lv_style_t speed_indic_style;
static lv_style_t speed_main_style;
static lv_style_t speed_label_style;
static lv_style_t speed_label_style_smpl;

static lv_obj_t *batt_arc = nullptr;
static lv_obj_t *batt_label = nullptr;
static lv_style_t batt_indic_style;
static lv_style_t batt_main_style;
static lv_style_t batt_label_style;
static lv_style_t batt_label_style_smpl;

static lv_obj_t *current_arc = nullptr;
static lv_obj_t *current_label = nullptr;
static lv_style_t current_indic_style;
static lv_style_t current_main_style;
static lv_style_t current_label_style;

static lv_obj_t *temp_arc = nullptr;
static lv_obj_t *temp_label = nullptr;
static lv_style_t temp_indic_style;
static lv_style_t temp_main_style;
static lv_style_t temp_label_style;
static lv_obj_t *speed_avg_bar = nullptr;
static lv_obj_t *speed_max_bar = nullptr;
static lv_obj_t *batt_max_bar = nullptr;
static lv_obj_t *batt_min_bar = nullptr;
static lv_obj_t *current_max_bar = nullptr;
static lv_obj_t *temp_max_bar = nullptr;
static lv_style_t max_bar_indic_style;
static lv_style_t max_bar_main_style;
static lv_style_t min_bar_indic_style;
static lv_style_t min_bar_main_style;

//Dashclock objects and styles
static lv_obj_t *dashtime = nullptr;
static lv_obj_t *wbatt = nullptr;
static lv_obj_t *trip = nullptr;
static lv_style_t dashtime_style;

//Clock objects and styles
static lv_obj_t *timeLabel = nullptr;
static lv_style_t time_style;
static lv_style_t timeLabel_style;
static lv_style_t timeLabel_bg_style;
static lv_obj_t *dateLabel = nullptr;
static lv_style_t dateLabel_style;

static lv_obj_t *battLabel = nullptr;
static lv_obj_t *battLabel_bg = nullptr;
static lv_style_t batt_fg_style;
static lv_style_t batt_bg_style;

// Text labels

// Bitmaps

/******************************
   LVGL Gui code
 ******************************/

/************************************
   Define LVGL default object styles
 ************************************/
void lv_define_styles_1(void) {
  int arclinew = 25;
  if (fulldash) {
    arclinew = 15; // line width of arc gauges
  }
  if (display_speedarc = false) {
    arclinew = 30;
  }
  //Speed arc and label

  lv_style_init(&speed_indic_style);
  lv_style_set_line_rounded(&speed_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_indic_style, LV_STATE_DEFAULT, arclinew);

  lv_style_init(&speed_main_style);
  lv_style_set_line_rounded(&speed_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_color(&speed_main_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_line_color(&speed_main_style, LV_STATE_DEFAULT, speed_bg_clr);
  if (display_speedarc = false) {
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

  //Max bar
  lv_style_init(&max_bar_indic_style);
  lv_style_set_line_rounded(&max_bar_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&max_bar_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&max_bar_indic_style, LV_STATE_DEFAULT, max_bar_clr);

  lv_style_init(&max_bar_main_style);
  lv_style_set_line_rounded(&max_bar_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&max_bar_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&max_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&max_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_opa(&max_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  //min bar
  lv_style_init(&min_bar_indic_style);
  lv_style_set_line_rounded(&min_bar_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&min_bar_indic_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_line_color(&min_bar_indic_style, LV_STATE_DEFAULT, min_bar_clr);

  lv_style_init(&min_bar_main_style);
  lv_style_set_line_rounded(&min_bar_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&min_bar_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_opa(&min_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_border_opa(&min_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_style_set_line_opa(&min_bar_main_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);


  // Clock
  lv_style_init(&dashtime_style);
  lv_style_set_text_color(&dashtime_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_text_font(&dashtime_style, LV_STATE_DEFAULT, &DIN1451_m_cond_28);
} //End Define LVGL default object styles

/***************************
    Create Dashboard objects
 ***************************/
void lv_speed_arc_1(void)
{
  /*Create speed gauge arc*/
  //Arc
  speed_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(speed_arc, LV_ARC_PART_INDIC, &speed_indic_style);
  lv_obj_add_style(speed_arc, LV_OBJ_PART_MAIN, &speed_main_style);
  lv_arc_set_bg_angles(speed_arc, 160, 20);
  lv_arc_set_range(speed_arc, 0, (wheeldata[15] + 5));
  lv_arc_set_value(speed_arc, wheeldata[1]);
  lv_obj_set_size(speed_arc, 268, 268);
  lv_obj_align(speed_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  if (display_speedarc = true) {
    //Max bar
    speed_max_bar = lv_arc_create(lv_scr_act(), NULL);
    lv_obj_add_style(speed_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
    lv_obj_add_style(speed_max_bar, LV_OBJ_PART_MAIN, &max_bar_main_style);
    lv_arc_set_bg_angles(speed_max_bar, 160, 20);
    lv_arc_set_range(speed_max_bar, 0, (wheeldata[15] + 5));
    lv_obj_set_size(speed_max_bar, 268, 268);
    lv_obj_align(speed_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

    //avg bar
    speed_avg_bar = lv_arc_create(lv_scr_act(), NULL);
    lv_obj_add_style(speed_avg_bar, LV_ARC_PART_INDIC, &min_bar_indic_style);
    lv_obj_add_style(speed_avg_bar, LV_OBJ_PART_MAIN, &min_bar_main_style);
    lv_arc_set_bg_angles(speed_avg_bar, 160, 20);
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
  lv_arc_set_type(batt_arc, LV_ARC_TYPE_REVERSE);
  lv_arc_set_bg_angles(batt_arc, 35, 145);
  lv_arc_set_angles(batt_arc, 35, 145);
  lv_arc_set_range(batt_arc, 0, 100);
  lv_arc_set_value(batt_arc, (100 - wheeldata[6]));
  lv_obj_set_size(batt_arc, 268, 268);
  lv_obj_align(batt_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //max bar
  batt_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(batt_max_bar, LV_OBJ_PART_MAIN, &max_bar_main_style);
  lv_arc_set_bg_angles(batt_max_bar, 35, 145);
  lv_arc_set_range(batt_max_bar, 0, 100);
  lv_obj_set_size(batt_max_bar, 268, 268);
  lv_obj_align(batt_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //min bar
  batt_min_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_min_bar, LV_ARC_PART_INDIC, &min_bar_indic_style);
  lv_obj_add_style(batt_min_bar, LV_OBJ_PART_MAIN, &min_bar_main_style);
  lv_arc_set_bg_angles(batt_min_bar, 35, 145);
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
  //Arc
  current_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_arc, LV_ARC_PART_INDIC, &current_indic_style);
  lv_obj_add_style(current_arc, LV_OBJ_PART_MAIN, &current_main_style);
  lv_arc_set_bg_angles(current_arc, 130, 230);
  lv_arc_set_range(current_arc, 0, maxcurrent);
  lv_arc_set_value(current_arc, wheeldata[3]);
  lv_obj_set_size(current_arc, 225, 225);
  lv_obj_align(current_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //Max bar
  current_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(current_max_bar, LV_OBJ_PART_MAIN, &max_bar_main_style);
  lv_arc_set_bg_angles(current_max_bar, 130, 230);
  lv_arc_set_range(current_max_bar, 0, maxcurrent);
  lv_obj_set_size(current_max_bar, 225, 225);
  lv_obj_align(current_max_bar, NULL, LV_ALIGN_CENTER, 0, 0);

  //Label
  current_label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(current_label, LV_OBJ_PART_MAIN, &current_label_style);
  char currentstring[4];
  dtostrf(wheeldata[3], 2, 0, currentstring);
  lv_label_set_text(current_label, currentstring);
  lv_label_set_align(current_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(current_label, current_arc, LV_ALIGN_CENTER, -64, 0);
}

void lv_temp_arc_1(void)
{
  /*Create temprature gauge arc*/
  //Arc
  temp_arc = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_arc, LV_ARC_PART_INDIC, &temp_indic_style);
  lv_obj_add_style(temp_arc, LV_OBJ_PART_MAIN, &temp_main_style);
  lv_arc_set_type(temp_arc, LV_ARC_TYPE_REVERSE);
  lv_arc_set_bg_angles(temp_arc, 310, 50);
  lv_arc_set_angles(temp_arc, 310, 50);
  lv_arc_set_range(temp_arc, 0, (crittemp + 10));
  lv_arc_set_value(temp_arc, ((crittemp + 10) - wheeldata[4]));
  lv_obj_set_size(temp_arc, 225, 225);
  lv_obj_align(temp_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //Max bar
  temp_max_bar = lv_arc_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_max_bar, LV_ARC_PART_INDIC, &max_bar_indic_style);
  lv_obj_add_style(temp_max_bar, LV_OBJ_PART_MAIN, &max_bar_main_style);
  lv_arc_set_bg_angles(temp_max_bar, 310, 50);
  lv_arc_set_range(temp_max_bar, 0, (crittemp + 10));
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
  lv_obj_add_style(trip, LV_OBJ_PART_MAIN, &current_label_style);
  lv_obj_align(trip, NULL, LV_ALIGN_IN_TOP_MID, 0, 25);
} //End Create Dashboard objects

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
    //speed_shake = lv_task_create(lv_speed_shake, 750, LV_TASK_PRIO_LOWEST, NULL);
    //lv_task_ready(speed_shake);
  } else {
    lv_style_set_line_color(&speed_indic_style, LV_STATE_DEFAULT, speed_fg_clr);
    lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, speed_fg_clr);
    //stop_speed_shake();
  }
  if (display_speedarc = true) {
    lv_obj_add_style(speed_arc, LV_ARC_PART_INDIC, &speed_indic_style);
    lv_arc_set_value(speed_arc, wheeldata[1]);

    lv_arc_set_angles(speed_max_bar, (160 + (max_speed * 200 / (wheeldata[15] + 5))), (163 + (max_speed * 200 / (wheeldata[15] + 5))));
    lv_arc_set_angles(speed_avg_bar, (160 + (avg_speed * 200 / (wheeldata[15] + 5))), (163 + (avg_speed * 200 / (wheeldata[15] + 5))));
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
    lv_obj_align(speed_label, speed_arc, LV_ALIGN_CENTER, 0, 0);
  }
}

void lv_batt_update(void) {
  if (wheeldata[6] < 10) {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[6] < 40) {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&batt_indic_style, LV_STATE_DEFAULT, batt_fg_clr);
    lv_style_set_text_color(&batt_label_style, LV_STATE_DEFAULT, batt_fg_clr);
  }
  lv_obj_add_style(batt_arc, LV_ARC_PART_INDIC, &batt_indic_style);
  lv_arc_set_value(batt_arc, (100 - wheeldata[6]));

  lv_arc_set_angles(batt_max_bar, (142 - (max_batt * 110 / 100)), (144 - (max_batt * 110 / 100)));
  lv_arc_set_angles(batt_min_bar, (142 - (min_batt * 110 / 100)), (144 - (min_batt * 110 / 100)));
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
  if (wheeldata[3] > (maxcurrent * 0.75)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    //current_shake = lv_task_create(lv_current_shake, 200, LV_TASK_PRIO_LOWEST, NULL);
  } else if (wheeldata[3] > (maxcurrent * 0.5)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    //stop_current_shake();
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
  lv_arc_set_value(current_arc, amps);

  lv_arc_set_angles(current_max_bar, (130 + (max_current * 100 / maxcurrent)), (133 + (max_current * 100 / maxcurrent)));

  lv_obj_add_style(current_label, LV_OBJ_PART_MAIN, &current_label_style);
  char currentstring[4];
  dtostrf(wheeldata[3], 2, 0, currentstring);
  lv_label_set_text(current_label, currentstring);
  lv_label_set_align(current_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(current_label, NULL, LV_ALIGN_CENTER, -64, 0);
}

void lv_temp_update(void) {
  // Set warning and alert colour
  if (wheeldata[4] > crittemp) {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    //temp_shake = lv_task_create(lv_temp_shake, 500, LV_TASK_PRIO_LOWEST, NULL);
  } else if (wheeldata[4] > warntemp) {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    //stop_temp_shake();
  } else {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, temp_fg_clr);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, temp_fg_clr);
    //stop_temp_shake();
  }
  lv_obj_add_style(temp_arc, LV_ARC_PART_INDIC, &temp_indic_style);
  lv_arc_set_value(temp_arc, ((crittemp + 10) - wheeldata[4]));

  lv_arc_set_angles(temp_max_bar, (50 - (max_temp * 100 / (crittemp + 10))), (53 - (max_temp * 100 / (crittemp + 10))));
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
    if (battLabel_bg != nullptr) {
      lv_label_set_text (battLabel_bg, LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP);
      lv_label_set_align(battLabel_bg, LV_LABEL_ALIGN_LEFT);
      lv_obj_align(battLabel_bg, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    }
    if (battLabel != nullptr) {
      if (watchbatt > 80) {
        lv_label_set_text (battLabel, LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP);
      } else if (watchbatt > 60) {
        lv_label_set_text (battLabel, LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP);
      } else if (watchbatt > 40) {
        lv_label_set_text (battLabel, LV_SYMBOL_STOP " " LV_SYMBOL_STOP " " LV_SYMBOL_STOP);
      } else if (watchbatt > 20 ) {
        lv_label_set_text (battLabel, LV_SYMBOL_STOP " " LV_SYMBOL_STOP);
      } else if (watchbatt > 5 ) {
        lv_label_set_text (battLabel, LV_SYMBOL_STOP);
      } else {
        lv_label_set_text (battLabel, LV_SYMBOL_CLOSE);
      }
      lv_label_set_align(battLabel, LV_LABEL_ALIGN_LEFT);
      lv_obj_align(battLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
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


/************************
   Task update functions
 ***********************/
static void lv_dash_task(lv_task_t * dash_task) {
  lv_speed_update();
  lv_batt_update();
  if (fulldash) {
    lv_current_update();
    lv_temp_update();
  }
}
static void lv_time_task(lv_task_t * time_task) {
  updateTime();
}
/*
  static void lv_current_shake(lv_task_t * current_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
  }
  static void lv_temp_shake(lv_task_t * temp_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
  }
*/

static void lv_speed_shake(lv_task_t * speed_shake) {
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->shake ();
  //delay(250);
  //ttgo->shake ();
}


/******************************************************
   Setup functions declared in the .h file and can be called
   from the .ino file
 ***************************************************/

void setup_timeGui(void) {
  Serial.println("setting up clock");
  lv_style_init(&timeLabel_bg_style);
  lv_style_set_radius(&timeLabel_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_style_set_bg_color(&timeLabel_bg_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK);
  lv_style_set_bg_opa(&timeLabel_bg_style, LV_OBJ_PART_MAIN, LV_OPA_COVER);
  lv_style_set_border_width(&timeLabel_bg_style, LV_OBJ_PART_MAIN, 0);
  lv_obj_t *view = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(view, 240, 240);
  lv_obj_add_style(view, LV_OBJ_PART_MAIN, &timeLabel_bg_style);

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
  lv_style_set_text_color(&batt_fg_style, LV_STATE_DEFAULT, watch_colour);

  lv_style_init(&batt_bg_style);
  lv_style_set_text_color(&batt_bg_style, LV_STATE_DEFAULT, watch_bg_colour);
  
  // battery indicator
  battLabel_bg = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(battLabel_bg, LV_OBJ_PART_MAIN, &batt_bg_style);
  lv_label_set_align(battLabel_bg, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(battLabel_bg, temp_arc, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  battLabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(battLabel, LV_OBJ_PART_MAIN, &batt_fg_style);
  lv_label_set_align(battLabel, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(battLabel, temp_arc, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  //run time update task
  time_task = lv_task_create(lv_time_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
  lv_task_ready(time_task);
}

void setup_LVGui(void) {
  Serial.println("setting up dashboard");
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
  }
}
/*
  void stop_current_shake() {
  if (current_shake != nullptr) {
    lv_task_del(current_shake);
  }
  }

  void stop_temp_shake() {
  if (temp_shake != nullptr) {
    lv_task_del(temp_shake);
  }
  }
*/
/*******************
   End LVGL GUI Code
 *******************/
