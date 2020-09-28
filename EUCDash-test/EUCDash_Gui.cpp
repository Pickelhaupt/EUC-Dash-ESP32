#include "twatch_config.h"
#include <Arduino.h>
#include <time.h>
#include "EUCDash_test.h"
#include "string.h"
#include <Ticker.h>
#include "BLEDevice.h"

int maxcurrent = 30;
int crittemp = 65;
int warntemp = 50;

extern float wheeldata[];
SemaphoreHandle_t dash_xSemaphore = xSemaphoreCreateMutex();

lv_task_t *dash_task = nullptr;
lv_task_t *time_task = nullptr;
/*
   Declare LVGL Dashboard objects and styles
*/

// Function declarations
//static void lv_dash_task(struct _lv_task_t *);
//static void lv_time_task(struct _lv_task_t *);
static void lv_dash_task(lv_task_t *dash_task);
static void lv_time_task(lv_task_t *time_task);
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

//Clock objects and styles
static lv_obj_t *timeLabel = nullptr;
static lv_style_t time_style;
static lv_style_t timeLabel_style;
static lv_style_t timeLabel_bg_style;

// Text labels

// Bitmaps

/******************************
   LVGL Gui code
 ******************************/

/************************************
   Define LVGL default object styles
 ************************************/
void lv_define_styles_1(void) {
  int arclinew = 13; // line width of arc gauges
  //Speed arc and label
  lv_style_init(&speed_indic_style);
  lv_style_set_line_rounded(&speed_indic_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_indic_style, LV_STATE_DEFAULT, arclinew);

  lv_style_init(&speed_main_style);
  lv_style_set_line_rounded(&speed_main_style, LV_STATE_DEFAULT, false);
  lv_style_set_line_width(&speed_main_style, LV_STATE_DEFAULT, arclinew);
  lv_style_set_bg_color(&speed_main_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_line_color(&speed_main_style, LV_STATE_DEFAULT, speed_bg_clr);

  lv_style_init(&speed_label_style);
  lv_style_set_text_color(&speed_label_style, LV_STATE_DEFAULT, speed_fg_clr);
  lv_style_set_text_font(&speed_label_style, LV_STATE_DEFAULT, &DIN1451_m_cond_120);

  lv_style_init(&speed_label_style_smpl);
  lv_style_set_text_color(&speed_label_style_smpl, LV_STATE_DEFAULT, speed_fg_clr);
  lv_style_set_text_font(&speed_label_style_smpl, LV_STATE_DEFAULT, &DIN1451_m_cond_150);

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

  lv_style_init(&batt_label_style_smpl);
  lv_style_set_text_font(&batt_label_style_smpl, LV_STATE_DEFAULT, &DIN1451_m_cond_80);

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
}

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

  //Label
  batt_label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(batt_label, LV_OBJ_PART_MAIN, &batt_label_style);
  char battstring[4];
  dtostrf(wheeldata[6], 2, 0, battstring);
  lv_label_set_text(batt_label, battstring);
  lv_label_set_align(batt_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(batt_label, batt_arc, LV_ALIGN_CENTER, 0, 75);
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
  lv_obj_set_size(current_arc, 227, 227);
  lv_obj_align(current_arc, NULL, LV_ALIGN_CENTER, 0, 0);

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
  lv_obj_set_size(temp_arc, 227, 227);
  lv_obj_align(temp_arc, NULL, LV_ALIGN_CENTER, 0, 0);

  //Label
  temp_label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_add_style(temp_label, LV_OBJ_PART_MAIN, &temp_label_style);
  char tempstring[4];
  dtostrf(wheeldata[4], 2, 0, tempstring);
  lv_label_set_text(temp_label, tempstring);
  lv_label_set_align(temp_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(temp_label, temp_arc, LV_ALIGN_CENTER, 64, 0);
}

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
  lv_obj_add_style(speed_arc, LV_ARC_PART_INDIC, &speed_indic_style);
  lv_arc_set_value(speed_arc, wheeldata[1]);

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

void lv_current_update(void) {
  // Set warning and alert colour
  if (wheeldata[3] > (maxcurrent * 0.75)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else if (wheeldata[3] > (maxcurrent * 0.5)) {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&current_indic_style, LV_STATE_DEFAULT, current_fg_clr);
    lv_style_set_text_color(&current_label_style, LV_STATE_DEFAULT, current_fg_clr);
  }
  lv_obj_add_style(current_arc, LV_ARC_PART_INDIC, &current_indic_style);
  lv_arc_set_value(current_arc, wheeldata[3]);

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
  } else if (wheeldata[4] > warntemp) {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  } else {
    lv_style_set_line_color(&temp_indic_style, LV_STATE_DEFAULT, temp_fg_clr);
    lv_style_set_text_color(&temp_label_style, LV_STATE_DEFAULT, temp_fg_clr);
  }
  lv_obj_add_style(temp_arc, LV_ARC_PART_INDIC, &temp_indic_style);
  lv_arc_set_value(temp_arc, ((crittemp + 10) - wheeldata[4]));

  lv_obj_add_style(temp_label, LV_OBJ_PART_MAIN, &temp_label_style);
  char tempstring[4];
  dtostrf(wheeldata[4], 2, 0, tempstring);
  lv_label_set_text(temp_label, tempstring);
  lv_label_set_align(temp_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(temp_label, NULL, LV_ALIGN_CENTER, 64, 0);
}

void updateTime()
{
  time_t now;
  struct tm  info;
  char buf[64];
  time(&now);
  localtime_r(&now, &info);
  strftime(buf, sizeof(buf), "%H:%M", &info);
  lv_label_set_text(timeLabel, buf);
  //lv_obj_align(timeLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
  lv_obj_align(timeLabel, NULL, LV_ALIGN_CENTER, 0, 0);
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->rtc->syncToRtc();
}

static void lv_dash_task(lv_task_t * dash_task) {
  lv_speed_update();
  lv_batt_update();
  lv_current_update();
  lv_temp_update();
}
static void lv_time_task(lv_task_t * time_task) {
  updateTime();
}

void setup_timeGui(void) {
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
  lv_obj_align(timeLabel, temp_arc, LV_ALIGN_CENTER, 0, 0);
  time_task = lv_task_create(lv_time_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
}

void setup_LVGui(void) {
  lv_define_styles_1();

  lv_speed_arc_1();
  lv_batt_arc_1();
  lv_current_arc_1();
  lv_temp_arc_1();
  //Create task -- update freq 4/s
  dash_task = lv_task_create(lv_dash_task, 250, LV_TASK_PRIO_MID, NULL);
}

void stop_time_task(){
  if (time_task != nullptr) {
    lv_task_del(time_task);
  }
}
void stop_dash_task(){
  if (dash_task != nullptr) {
    lv_task_del(dash_task);
  }
}
/*******************
   End LVGL GUI Code
 *******************/
