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
#define CPU_FREQ_LOW     40
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

struct Wheel_constants {
  byte maxcurrent;
  byte crittemp;
  byte warntemp;
  byte battvolt;
  byte battwarn;
};

extern struct Wheel_constants wheelconst;

//Global function declarations
void setup_timeGui();
void setup_LVGui();
void updateBatteryIcon(lv_icon_battery_t index);
void updateBatteryLevel();
void updateTime();
void stop_time_task();
void stop_dash_task();
void stop_speed_shake();
void stop_current_shake();
void stop_temp_shake();
void setbrightness();
void decodeKS(byte KSData[]);
void initks();
void ks_ble_request(byte);
void writeBLE (byte*, int);

extern unsigned int screenTimeout;
extern unsigned int defaultScreenTimeout;
extern unsigned int ridingScreenTimeout;
extern unsigned int defaultCpuFrequency;
extern int ride_mode;
extern boolean connected;
extern bool displayOff;
extern int maxcurrent;
extern int crittemp;
extern int warntemp;
extern float max_speed;
extern float avg_speed;
extern float max_batt;
extern float min_batt;
extern float max_current;
extern float max_temp;
extern String wheelmodel;
extern String Wheel_brand;

extern "C" {
  extern void lv_keyboard_def_event_cb(lv_obj_t * kb, lv_event_t event);
}
#endif /*__GUI_H */
