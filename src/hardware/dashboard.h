/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
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
#ifndef _DASHBOARD_H
    #define _DASHBOARD_H

    #include "callback.h"

    #define DASHBOARD_JSON_CONFIG_FILE    "/dashboard.json" /** @brief defines json config file name */

    #define DASHCTL_BARS       _BV(0)          /** @brief event mask dashboard bar display, callback arg is (uint32_t*) */
    #define DASHCTL_CURRENT    _BV(1)          /** @brief event mask dashboard current display, callback arg is (uint32_t*) */

    /**
     * @brief dashboard config structure
     */
    typedef struct {
        bool enable=true;
    } dashboard_config_t;

    enum { 
        DASHBOARD_LIGHTS,
        DASHBOARD_BARS,
        DASHBOARD_CURRENT,
        DASHBOARD_SIMPLE,
        DASHBOARD_IMPDIST,
        DASHBOARD_IMPTEMP,
        DASHBOARD_CONFIG_NUM
    };

    /*
     * @brief display config structure
     */
    
    //typedef struct {
    //    bool display_bars = true;                       /** @brief display brightness */
    //    bool display_current = false;                   /** @brief display time out */                      
    //    bool default_simple = false;                    /** @brief Jump to the simple dash tile on wakeup when connected */
    //    bool imp_distance = false;                      /** @brief Set distance and speed units to imperial */
    //} dashboard_config_t;


    /**
     * @brief setup dashboard
     * 
     */
    void dashboard_setup( void );
    /**
     * @brief display loop
     * 
     * @param   ttgo    pointer to an TTGOClass
     */
    void dashboard_loop( void );
    /**
     * @brief save config for dashboard to spiffs
     */
    void dashboard_save_config( void );
    /**
     * @brief read config for dashboard from spiffs
     */
    void dashboard_read_config( void );
    /**
     * @brief read the config for a specific dashboard setting
     * 
     * @param   config     configitem: DASHBOARD_BARS, DASHBOARD_CURRENT, DASHBOARD_SIMPLE, DASHBOARD_IMPDIST, DASHBOARD_IMPTEMP
     * 
     * @return  bool
     */
    bool dashboard_get_config( int config );
    /**
     * @brief set config item
     * 
     * @param   config     configitem: DASHBOARD_BARS, DASHBOARD_CURRENT, DASHBOARD_SIMPLE, DASHBOARD_IMPDIST, DASHBOARD_IMPTEMP
     * @param   bool    true or false
     */
    void dashboard_set_config( int config, bool enable );
    /**
     * @brief registers a callback function which is called on a corresponding event
     * 
     * @param   event           possible values: DASHCTL_BARS and DASHCTL_CURRENT
     * @param   callback_func   pointer to the callback function
     * @param   id              program id
     * 
     * @return  true if success, false if failed
     */
    bool dashboard_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id );


#endif // _DASHBOARD_H