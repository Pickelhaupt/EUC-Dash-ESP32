// Function declarations
static void lv_dash_task(lv_task_t *dash_task);
static void lv_time_task(lv_task_t *time_task);
static void lv_speed_shake(lv_task_t *speed_shake);
static void lv_current_shake(lv_task_t *current_shake);
static void lv_temp_shake(lv_task_t *temp_shake);

//task declarations
lv_task_t *dash_task = nullptr;
lv_task_t *time_task = nullptr;
lv_task_t *speed_shake = nullptr;
lv_task_t *current_shake = nullptr;
lv_task_t *temp_shake = nullptr;

/**********************************************
   Define custom colours for the EUC gauges
   Edit if you want to change colour scheme
   All arcs will turn yellow and red when there
   is a warning or critical state
 **********************************************/
// Gauge colours
static lv_color_t current_bg_clr = lv_color_make(0x25, 0x25, 0x25); //Current gauge arc background (default = dark gray)
static lv_color_t current_fg_clr = lv_color_make(0xff, 0xff, 0xff); //Current gauge arc background (default = white)
static lv_color_t speed_bg_clr = lv_color_make(0x05, 0x2a, 0x05); //Speed gauge arc background (default = dark green)
static lv_color_t speed_fg_clr = lv_color_make(0x00, 0xfa, 0x0f); //Speed gauge indicator color (default = green)
static lv_color_t batt_bg_clr = lv_color_make(0x05, 0x2a, 0x05); //Battery gauge arc background (default = dark green)
static lv_color_t batt_fg_clr = lv_color_make(0x00, 0xfa, 0x0f); //Battery gauge indicator color (default = green)
static lv_color_t temp_bg_clr = lv_color_make(0x05, 0x05, 0x3a); //Temperature gauge arc background (dark blue)
static lv_color_t temp_fg_clr = lv_color_make(0x2a, 0x1f, 0xff); //Temperature gauge indicator color (light blue)
//Misc colours
static lv_color_t ride_mode_clr = lv_color_make(0xFF, 0x00, 0xFF); //The H M S ride mode indicator (Magenta)
static lv_color_t max_bar_clr = lv_color_make(0xFF, 0x00, 0xFF); // (Magenta)
static lv_color_t min_bar_clr = lv_color_make(0x2a, 0x1f, 0xff); // (light blue)
static lv_color_t watch_info_colour = lv_color_make(0xe0, 0xe0, 0xe0); //Watch battery level and time (Gray)
//Disconnected time display colour
static lv_color_t watch_colour = lv_color_make(0xFF, 0x00, 0x00); // (Red)
static lv_color_t watch_bg_colour = lv_color_make(0xAF, 0x00, 0x00); // (Dark Red)

/***********************************************************
   Declare custom fonts, TTF fonts can be converted to C at:
   https://lvgl.io/tools/fontconverter
   All custom fonts reside in the sketch directory
 ***********************************************************/
LV_FONT_DECLARE(DIN1451_m_cond_24);
LV_FONT_DECLARE(DIN1451_m_cond_28);
LV_FONT_DECLARE(DIN1451_m_cond_36);
LV_FONT_DECLARE(DIN1451_m_cond_44);
LV_FONT_DECLARE(DIN1451_m_cond_66);
LV_FONT_DECLARE(DIN1451_m_cond_72);
LV_FONT_DECLARE(DIN1451_m_cond_80);
LV_FONT_DECLARE(DIN1451_m_cond_120);
LV_FONT_DECLARE(DIN1451_m_cond_150);
LV_FONT_DECLARE(DIN1451_m_cond_180);

/*
   Declare LVGL Dashboard objects and styles
*/

// Arc gauges and labels
//Speed
static lv_obj_t *speed_arc = nullptr;
static lv_obj_t *speed_label = nullptr;
static lv_style_t speed_indic_style;
static lv_style_t speed_main_style;
static lv_style_t speed_label_style;
//Battery
static lv_obj_t *batt_arc = nullptr;
static lv_obj_t *batt_label = nullptr;
static lv_style_t batt_indic_style;
static lv_style_t batt_main_style;
static lv_style_t batt_label_style;
//Current
static lv_obj_t *current_arc = nullptr;
static lv_obj_t *current_label = nullptr;
static lv_style_t current_indic_style;
static lv_style_t current_main_style;
static lv_style_t current_label_style;
//Temperature
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

//Max avg and min bars
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
