/****************************************************************************
 *   Copyright  2020  Jesper Ortlund
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
#ifndef _EUCDASH_SETTINGS_H
    #define _EUCDASH_SETTINGS_H

    #include <TTGO.h>

    #define MAX_MENU_ITEMS      12

    /**
     * @brief structure to hold menu item data
     * 
     * @param   cont (lv_obj_t) the lvgl container object for the menu item
     * @param   label (lv_obj_t) the lvgl label object for the menu item
     * @param   icon
     * @param   arrow
     */
    typedef struct {
        lv_obj_t *cont;
        lv_obj_t *label;
        lv_obj_t *icon;
        lv_obj_t *arrow;
    } eucdash_settings_item_t;

    /**
     * @brief set up the eucdash settings tile
     */
    void eucdash_settings_tile_setup( void );
    /**
     * @brief register a menu item to the eucdash settings menu
     * 
     * @param   icon the icon representing the menu item, should be 32x32 px
     * @param   event_cb the callback function to be called when the item is clicked
     * @param   item_label the label text to be shown on the menu item, 2 lines of approx. 16 chars
     * @return  menu entry number
     */
    uint32_t eucdash_settings_register_menu_item(const lv_img_dsc_t *icon, lv_event_cb_t event_cb, const char *item_label);

    uint32_t eucdash_get_tile_num();

    uint32_t eucdash_get_submenu_tile_num();

#endif // _EUCDASH_SETTINGS_TILE_H