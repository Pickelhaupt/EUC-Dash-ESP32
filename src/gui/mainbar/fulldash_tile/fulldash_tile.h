/****************************************************************************
 *   Copyright  2020  Jesper Ortlund
 *   Partly derived from My-ttgo-watch by Dirk Brosswick
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
#ifndef _FULLDASH_TILE_H
#define _FULLDASH_TILE_H

#include <TTGO.h>

/**********************************************
   Define custom colours for the EUC gauges
   Edit if you want to change colour scheme
   All arcs will turn yellow and red when there
   is a warning or critical state
 **********************************************/
// Gauge colours
static lv_color_t current_bg_clr = lv_color_make(0x25, 0x25, 0x25); //Current gauge arc background (default = dark gray)
static lv_color_t current_fg_clr = lv_color_make(0xff, 0xff, 0xff); //Current gauge arc background (default = white)
static lv_color_t speed_bg_clr = lv_color_make(0x05, 0x2a, 0x05);   //Speed gauge arc background (default = dark green)
static lv_color_t speed_fg_clr = lv_color_make(0x00, 0xfa, 0x0f);   //Speed gauge indicator color (default = green)
static lv_color_t batt_bg_clr = lv_color_make(0x05, 0x2a, 0x05);    //Battery gauge arc background (default = dark green)

// change default bg col for batt arc MJR 14-OCT-2020 -- could not get this to compile, reverted to original CJO 18-10
//static lv_color_t batt_light_bg_clr = ( fulldash ? lv_color_make(0x05, 0x4a, 0x15) : lv_color_make(0xff, 0x00, 0x00) ); // green or red
static lv_color_t batt_light_bg_clr = lv_color_make(0x05, 0x4a, 0x15); //Battery gauge arc background (default = dark green)

static lv_color_t batt_fg_clr = lv_color_make(0x00, 0xfa, 0x0f);        //Battery gauge indicator color (default = green)
static lv_color_t temp_bg_clr = lv_color_make(0x05, 0x05, 0x3a);        //Temperature gauge arc background (dark blue)
static lv_color_t temp_fg_clr = lv_color_make(0x2a, 0x1f, 0xff);        //Temperature gauge indicator color (light blue)
static lv_color_t arc_warning_colour = lv_color_make(0x3a, 0x3a, 0x05); //Colour for warnings (Dark yellow)
static lv_color_t arc_crit_colour = lv_color_make(0x3a, 0x05, 0x05);    //Colour for critical states (Dark red)
//Misc colours
static lv_color_t ride_mode_clr = lv_color_make(0xFF, 0x00, 0xFF);     //The H M S ride mode indicator (Magenta)
static lv_color_t max_bar_clr = lv_color_make(0xFF, 0x00, 0xFF);       // (Magenta)
static lv_color_t min_bar_clr = lv_color_make(0x2a, 0x1f, 0xff);       // (light blue)
static lv_color_t regen_bar_clr = lv_color_make(0x00, 0xfa, 0x0f);     // (green)
static lv_color_t watch_info_colour = lv_color_make(0xe0, 0xe0, 0xe0); //Watch battery level and time (Gray)
static lv_color_t warning_colour = lv_color_make(0xff, 0xff, 0x00);    //Colour for warnings (Yellow)
static lv_color_t crit_colour = lv_color_make(0xff, 0x00, 0x00);       //Colour for critical states (RED)
//Disconnected time display colour
static lv_color_t watch_colour = lv_color_make(0xFF, 0x00, 0x00);    // (Red)
static lv_color_t watch_bg_colour = lv_color_make(0xAF, 0x00, 0x00); // (Dark Red)


/***********************************************************
   Declare custom fonts, TTF fonts can be converted to C at:
   https://lvgl.io/tools/fontconverter
   All custom fonts reside in the font directory
 ***********************************************************/
LV_FONT_DECLARE(DIN1451_m_cond_24);
LV_FONT_DECLARE(DIN1451_m_cond_28);
LV_FONT_DECLARE(DIN1451_m_cond_36);
LV_FONT_DECLARE(DIN1451_m_cond_44);
LV_FONT_DECLARE(DIN1451_m_cond_66);
LV_FONT_DECLARE(DIN1451_m_cond_120);
LV_FONT_DECLARE(DIN1451_m_cond_180);

// Text labels

// Bitmaps

//Functions
/**
* @brief setup the full dashboard tile
*/
void fulldash_tile_setup(void);
//Functions
/**
* @brief reload the full dashboard tile
*/
void fulldash_tile_reload ( void );
/**
* @brief get the tile number for the full dashboard tile
* 
* @return  tile number
*/
uint32_t fulldash_get_tile (void);
/**
* @brief update speed arc and label
*/
void fulldash_speed_update(float current_speed, float warn_speed, float tiltback_speed, float top_speed, float avg_speed);
/**
* @brief update battery arc and label
*/
void fulldash_batt_update(float current_battpct, float min_battpct, float max_battpct);
/**
* @brief update current arc and label
*/
void fulldash_current_update(float current_current, byte maxcurrent, float min_current, float max_current);
/**
* @brief update temperature arc and label
*/
void fulldash_temp_update(float current_temp, byte warn_temp, byte crit_temp, float max_temp);
/**
* @brief update overlay
*/
void fulldash_overlay_update();
/**
* @brief display current alert icon on the full dashboard
* @param enabled set to true to enable
*/
void fulldash_current_alert(bool enabled);
/**
* @brief display battery alert icon on the full dashboard
* @param enabled set to true to enable
*/
void fulldash_batt_alert(bool enabled);
/**
* @brief display temperature alert icon on the full dashboard
* @param enabled set to true to enable
*/
void fulldash_temp_alert(bool enabled);
/**
* @brief display fan running notification dashboard
* @param enabled set to true to enable
*/
void fulldash_fan_indic(bool enabled);

//variable declarations

extern bool fulldash_active;

#endif // _FULLDASH_TILE_H