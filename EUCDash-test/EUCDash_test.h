/*
  GPLv3 Jesper Ortlund Sept 2020
  Derived from:
  agoodwatch Copyright (c) 2020 Alex Goodyear
  Derived from:
  Simple watch in T-watch example library Copyright (c) 2019 Lewis He
*/
#ifndef __GUI_H
#define __GUI_H

#define STR(_s) XSTR(_s)
#define XSTR(_s) #_s

#define THIS_VERSION_ID  0.1
#define THIS_VERSION_STR "Ver " STR(THIS_VERSION_ID)

/*
   Use a time-zone string from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   Courtesy of Andreas Spiess NTP example https://github.com/SensorsIot and his YouTube channel
   https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ
*/
//Some examples:
// #define RTC_TIME_ZONE   "GMT0BST,M3.5.0/1,M10.5.0"  // Europe/London/Dublin/Lisbon etc.
// #define RTC_TIME_ZONE   "EET-2EEST,M3.5.0/3,M10.5.0/4" // Europe/Athens/Helsinki/Sofia etc.
// #define RTC_TIME_ZONE   "CET-1CEST,M3.5.0,M10.5.0/3" // Europe/Paris/Berlin/Stockholm/Amsterdam/Rome etc.
// #define RTC_TIME_ZONE   "EST5EDT,M3.2.0,M11.1.0" // America/New_York/Montreal etc.
// #define RTC_TIME_ZONE   "PST8PDT,M3.2.0,M11.1.0" // America/Los_Angeles
// #define RTC_TIME_ZONE   "AEST-10AEDT,M10.1.0,M4.1.0/3" // Australia/Sydney/Melbourne/Hobart
// #define RTC_TIME_ZONE   "ACST-9:30ACDT,M10.1.0,M4.1.0/3" // Australia/Adelade
// #define RTC_TIME_ZONE   "AWST-8" // Australia/Perth
#define RTC_TIME_ZONE   "AEST-10" // Australia/Brisbane

/*
   The number of milliseconds of inactivity before the watch goes to sleep. Every tap or swipe on the
   screen will reset the internal timer.
*/
#define DEFAULT_SCREEN_TIMEOUT  15*1000    // when the watch is disconnected from the whee;
#define RIDING_SCREEN_TIMEOUT  60*1000    // Long timeout when connected to the wheel

#define CPU_FREQ_MIN     10
#define CPU_FREQ_NORM    80
#define CPU_FREQ_WIFI    80
#define CPU_FREQ_BLE     80  //to avoid BLE notification issues
#define CPU_FREQ_MEDIUM 160
#define CPU_FREQ_MAX    240

//Not used, left is as a placeholder in case I would want a battery icon on the watch display
typedef enum {
  LV_ICON_BAT_EMPTY,
  LV_ICON_BAT_1,
  LV_ICON_BAT_2,
  LV_ICON_BAT_3,
  LV_ICON_BAT_FULL,
  LV_ICON_CHARGE,
  LV_ICON_CALCULATION = LV_ICON_BAT_FULL
} lv_icon_battery_t;

typedef enum {
  LV_STATUS_BAR_BATTERY_LEVEL = 0,
  LV_STATUS_BAR_BATTERY_ICON = 1,
  LV_STATUS_BAR_WIFI = 2,
  LV_STATUS_BAR_BLUETOOTH = 3,
} lv_icon_status_bar_t;

/*
** I know it is a bit odd to split col and row but it simplifies the
** initialisation declarations in the code.
*/
typedef struct
{
  int        col;             // x
  int      (*create)   (int);
  int        row;             // y
  int      (*onEntry)  (int);
  int      (*onExit)   (int);
  int      (*onUpdate) (int);
  lv_obj_t  *tile;
  void      *data;
} TileDesc_t;

void setup_timeGui();
void setup_LVGui();
void updateBatteryIcon(lv_icon_battery_t index);
void updateBatteryLevel();
void updateTime();
void stop_time_task();
void stop_dash_task();

/**********************************************
   Define custom colours for the EUC gauges
   Edit if you want to change colour scheme
   All arcs will turn yellow and red when there
   is a warning or critical state
 **********************************************/
// Gauge colours
static lv_color_t current_bg_clr = lv_color_make(0x05, 0x05, 0x05); //Current gauge arc background (default = dark gray)
static lv_color_t current_fg_clr = lv_color_make(0xff, 0xff, 0xff); //Current gauge arc background (default = white)
static lv_color_t speed_bg_clr = lv_color_make(0x00, 0x0a, 0x00); //Speed gauge arc background (default = dark green)
static lv_color_t speed_fg_clr = lv_color_make(0x00, 0xfa, 0x0f); //Speed gauge indicator color (default = green)
static lv_color_t batt_bg_clr = lv_color_make(0x00, 0x0a, 0x00); //Speed gauge arc background (default = dark green)
static lv_color_t batt_fg_clr = lv_color_make(0x00, 0xfa, 0x0f); //Speed gauge indicator color (default = green)
static lv_color_t temp_bg_clr = lv_color_make(0x05, 0x05, 0x0a); //Temperature gauge arc background
static lv_color_t temp_fg_clr = lv_color_make(0x2a, 0x1f, 0xff); //Temperature gauge indicator color
//Misc colours
static lv_color_t ride_mode_colour = lv_color_make(0xFF, 0xFF, 0x00); //The H M S ride mode indicator (Magenta)
static lv_color_t watch_info_colour = lv_color_make(0xB0, 0xB0, 0xB0); //Watch battery level and time (Gray)
//Disconnected time display colour
static lv_color_t watch_colour = lv_color_make(0xFF, 0x00, 0x00); // (Red)

/***********************************************************
   Declare custom fonts, TTF fonts can be converted to C at:
   https://lvgl.io/tools/fontconverter
   All custom fonts reside in the sketch directory
 ***********************************************************/

LV_FONT_DECLARE(DIN1451_m_cond_24);
LV_FONT_DECLARE(DIN1451_m_cond_36);
LV_FONT_DECLARE(DIN1451_m_cond_44);
LV_FONT_DECLARE(DIN1451_m_cond_66);
LV_FONT_DECLARE(DIN1451_m_cond_72);
LV_FONT_DECLARE(DIN1451_m_cond_80);
LV_FONT_DECLARE(DIN1451_m_cond_120);
LV_FONT_DECLARE(DIN1451_m_cond_150);

extern unsigned int screenTimeout;
extern unsigned int defaultScreenTimeout;
extern unsigned int ridingScreenTimeout;
extern unsigned int defaultCpuFrequency;
extern int ride_mode;
extern boolean connected;
extern int maxcurrent;
extern int crittemp;
extern int warntemp;

extern "C" {
  extern void lv_keyboard_def_event_cb(lv_obj_t * kb, lv_event_t event);
}
#endif /*__GUI_H */
