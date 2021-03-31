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
#include "config.h"
#include <TTGO.h>

#include "dashboard.h"
#include "powermgm.h"
#include "gui/gui.h"
#include "gui/mainbar/fulldash_tile/fulldash_tile.h"

#include "json_psram_allocator.h"

callback_t *dashboard_callback = NULL;

dashboard_config_t dashboard_config[ DASHBOARD_CONFIG_NUM ];

void dashboard_setup( void ) {
    dashboard_read_config();
}

void dashboard_save_config( void ) {
    fs::File file = SPIFFS.open( DASHBOARD_JSON_CONFIG_FILE, FILE_WRITE );

    if (!file) {
        log_e("Can't open file: %s!", DASHBOARD_JSON_CONFIG_FILE );
    }
    else {
        SpiRamJsonDocument doc( 1000 );

        doc["lights"] = dashboard_config[ DASHBOARD_LIGHTS ].enable;
        doc["bars"] = dashboard_config[ DASHBOARD_BARS ].enable;
        doc["simple"] = dashboard_config[ DASHBOARD_SIMPLE ].enable;
        doc["medium"] = dashboard_config[ DASHBOARD_MEDIUM ].enable;
        doc["full"] = dashboard_config[ DASHBOARD_FULL ].enable;
        doc["timedisplay"] = dashboard_config[ DASHBOARD_TIME ].enable;
        doc["impdist"] = dashboard_config[ DASHBOARD_IMPDIST ].enable;
        doc["imptemp"] = dashboard_config[ DASHBOARD_IMPTEMP ].enable;

        if ( serializeJsonPretty( doc, file ) == 0) {
            log_e("Failed to write config file");
        }
        doc.clear();
    }
    file.close();
}

void dashboard_read_config( void ) {
    fs::File file = SPIFFS.open( DASHBOARD_JSON_CONFIG_FILE, FILE_READ );
    if (!file) {
        log_e("Can't open file: %s!", DASHBOARD_JSON_CONFIG_FILE );
    }
    else {
        int fs = 0;
        int filesize = file.size();
        if (file.size() == 0) fs = 1000;
        SpiRamJsonDocument doc( (filesize * 2) + fs );

        DeserializationError error = deserializeJson( doc, file );
        if ( error ) {
            log_e("update check deserializeJson() failed: %s", error.c_str() );
            /*
            SPIFFS.end();
            delay(100);
            SPIFFS.format();
            log_e("SPIFFS formatted");
            SPIFFS.begin();
            */
        }
        else {
            dashboard_config[ DASHBOARD_LIGHTS ].enable = doc["lights"] | true;
            dashboard_config[ DASHBOARD_BARS ].enable = doc["bars"] | true;
            dashboard_config[ DASHBOARD_SIMPLE ].enable = doc["simple"] | false;
            dashboard_config[ DASHBOARD_MEDIUM ].enable = doc["medium"] | false;
            dashboard_config[ DASHBOARD_FULL ].enable = doc["full"] | true;
            dashboard_config[ DASHBOARD_TIME ].enable = doc["timedisplay"] | false;
            dashboard_config[ DASHBOARD_IMPDIST ].enable = doc["impdist"] | false;
            dashboard_config[ DASHBOARD_IMPTEMP ].enable = doc["imptemp"] | false;
        }        
        doc.clear();
    }
    file.close();
}

bool dashboard_get_config( int config ) {
    if ( config < DASHBOARD_CONFIG_NUM ) {
        return( dashboard_config[ config ].enable );
    }
    return false;
}

void dashboard_set_config( int config, bool enable ) {
    if ( config < DASHBOARD_CONFIG_NUM ) {
        dashboard_config[ config ].enable = enable;
        //simpledash_tile_reload();
    }
}

void dashboard_save_and_reload(void) {
    dashboard_save_config();
    dashboard_tile_reload();
}