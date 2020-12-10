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

#ifndef _SIMPLEDASH_TILE_H
#define _SIMPLEDASH_TILE_H

#include <TTGO.h>

/**********************************************
   Define custom colours for the EUC gauges
   Edit if you want to change colour scheme
   All arcs will turn yellow and red when there
   is a warning or critical state
 **********************************************/
// Gauge colours
static lv_color_t sd_current_bg_clr = lv_color_make(0x25, 0x25, 0x25);    //Current gauge arc background (default = dark gray)
static lv_color_t sd_current_fg_clr = lv_color_make(0xff, 0xff, 0xff);    //Current gauge arc background (default = white)
static lv_color_t sd_speed_bg_clr = lv_color_make(0x05, 0x2a, 0x05);      //Speed gauge arc background (default = dark green)
static lv_color_t sd_speed_fg_clr = lv_color_make(0x00, 0xfa, 0x0f);      //Speed gauge indicator color (default = green)
static lv_color_t sd_batt_bg_clr = lv_color_make(0x05, 0x2a, 0x05);       //Battery gauge arc background (default = dark green)
static lv_color_t sd_batt_light_bg_clr = lv_color_make(0x05, 0x4a, 0x15); //Battery gauge arc background (default = dark green)
static lv_color_t sd_batt_fg_clr = lv_color_make(0x00, 0xfa, 0x0f);       //Battery gauge indicator color (default = green)
static lv_color_t sd_temp_bg_clr = lv_color_make(0x05, 0x05, 0x3a);       //Temperature gauge arc background (dark blue)
static lv_color_t sd_temp_fg_clr = lv_color_make(0x2a, 0x1f, 0xff);       //Temperature gauge indicator color (light blue)
static lv_color_t sd_arc_warning_colour = lv_color_make(0x3a, 0x3a, 0x05); //Colour for warnings (Dark yellow)
static lv_color_t sd_arc_crit_colour = lv_color_make(0x3a, 0x05, 0x05);   //Colour for critical states (Dark red)
//Misc colours
static lv_color_t sd_ride_mode_clr = lv_color_make(0xFF, 0x00, 0xFF);     //The H M S ride mode indicator (Magenta)
static lv_color_t sd_max_bar_clr = lv_color_make(0xFF, 0x00, 0xFF);       // (Magenta)
static lv_color_t sd_min_bar_clr = lv_color_make(0x2a, 0x1f, 0xff);       // (light blue)
static lv_color_t sd_regen_bar_clr = lv_color_make(0x00, 0xfa, 0x0f);     // (green)
static lv_color_t sd_warning_colour = lv_color_make(0xff, 0xff, 0x00);    //Colour for warnings (Yellow)
static lv_color_t sd_crit_colour = lv_color_make(0xff, 0x00, 0x00);       //Colour for critical states (RED)

//Functions
/**
* @brief Setup all widgets on the simple dash
*/
void simpledash_tile_setup(void);
/**
* @brief Get the tile number of the simpe dash
*/
uint32_t simpledash_get_tile (void);
/**
* @brief reload the simpledash tile
*/
void simpledash_tile_reload ( void );
/**
* @brief update speed arc and label
* @param current_speed current speed as reported by the wheel
* @param warn_speed the speed when the last speed alarm is triggered (alarm3)
* @param tiltback_speed The speed when the wheels tiltback feature is activated
*/
void simpledash_speed_update(float current_speed, float warn_speed, float tiltback_speed);
/**
* @brief update battery arc and label
*/
void simpledash_batt_update(float current_battpct, float min_battpct, float max_battpct);
/**
* @brief update current arc and label
*/
void simpledash_current_update(float current_current, byte maxcurrent, float min_current, float max_current);
/**
* @brief display current alert icon on the simple dashboard
* @param enabled set to true to enable
*/
void simpledash_current_alert(bool enabled);
/**
* @brief display battery alert icon on the simple dashboard
* @param enabled set to true to enable
*/
void simpledash_batt_alert(bool enabled);
/**
* @brief display temperature alert icon on the simple dashboard
* @param enabled set to true to enable
*/
void simpledash_temp_alert(bool enabled);
/**
* @brief display fan running notification on the simple dashboard
* @param enabled set to true to enable
*/
void simpledash_fan_indic(bool enabled);

//variable declarations
extern bool simpledash_active;

#endif // _SIMPLEDASH_TILE_H