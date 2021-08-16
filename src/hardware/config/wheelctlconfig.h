#ifndef _WHEELCTLCONFIG_H
    #define _WHEELCTLCONFIG_H

    #include "config.h"
    #include "basejsonconfig.h"

    #define WHEELCTLCONFIG_JSON_FILE    "/wheelctlconfig.json" /** @brief defines json config file name */
    #define CURRENT_TRIP_JSON_FILE    "/c-tripsave.json" /** @brief defines json config file name */
    #define LAST_TRIP_JSON_FILE    "/l-tripsave.json" /** @brief defines json config file name */
    #define TOTAL_TRIP_JSON_FILE    "/t-tripsave.json" /** @brief defines json config file name */

    /**
     * @brief persistent trip data structure
     */
    class tripdata_t : public BaseJsonConfig {
        public:
        tripdata_t();
        long timestamp = 0;
        float trip = 0.0;
        float ride_time = 0;
        float avg_speed = 0;
        float max_speed = 0;
        float max_current = 0;
        float max_regen_current = 0;
        float max_power = 0;
        float min_battery = 100;
        float max_battery = 0;
        float max_temperature = 0;
        float consumed_energy = 0;
        float trip_economy = 0;

        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad(JsonDocument& document);
        virtual bool onSave(JsonDocument& document);
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 2000; }
    } ;


    /**
     * @brief wifictl config structure
     */
    class wheelctlconfig_t : public BaseJsonConfig {
        public:
        wheelctlconfig_t();
        byte lights_mode = 0x00;
        byte flash_mode = 0x00;
        byte lights_on_mode = 0x00;
        bool save_light_state = false;
        bool cycle_beamtype = false;
        bool haptic = true;
        bool horn_while_riding = false;

        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad(JsonDocument& document);
        virtual bool onSave(JsonDocument& document);
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 2000; }
    };

#endif // _WHEELCTLCONFIG_H