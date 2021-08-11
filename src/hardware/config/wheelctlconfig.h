#ifndef _WHEELCTLCONFIG_H
    #define _WHEELCTLCONFIG_H

    #include "config.h"
    #include "basejsonconfig.h"

    #define WHEELCTL_JSON_CONFIG_FILE    "/wheelctl.json" /** @brief defines json config file name */
    #define CURRENT_TRIP_JSON_FILE    "/c-tripsave.json" /** @brief defines json config file name */
    #define LAST_TRIP_JSON_FILE    "/l-tripsave.json" /** @brief defines json config file name */
    #define TOTAL_TRIP_JSON_FILE    "/t-tripsave.json" /** @brief defines json config file name */

    /**
     * @brief persistent tripdata structure
     */
    typedef struct {
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
    } trip_data_t;

        /**
     * @brief wheel config structure
     */
    typedef struct {
        bool enable=true;
    } wheelctl_config_t;

    enum {  
        WHEELCTL_CONFIG_LED,            //toggle leds when toggling light
        WHEELCTL_CONFIG_HORN,           //long press on dash activates horn if speed > 3kmh
        WHEELCTL_CONFIG_HAPTIC,         //Enable haptic feedback for alerts
        WHEELCTL_CONFIG_LIGHTS_OFF,     //turn off lights when connecting to wheel
        WHEELCTL_CONFIG_NUM     //number of wheel configuration parameters
    };

    /**
     * @brief network list structure
     */
    typedef struct {
        char ssid[64]="";
        char password[64]="";
    } wifictl_networklist;

    /**
     * @brief wifictl config structure
     */
    class wifictl_config_t : public BaseJsonConfig {
        public:
        wifictl_config_t();
        bool autoon = true;                                     /** @brief enable on auto on/off an wakeup and standby */
        bool enable_on_standby = false;                         /** @brief enable on standby */
        #ifdef ENABLE_WEBSERVER
            bool webserver = false;                             /** @brief enable on webserver */
        #endif
        #ifdef ENABLE_FTPSERVER
            bool ftpserver = false;                             /** @brief enable on ftpserver */
            char ftpuser[32] = FTPSERVER_USER;                  /** @brief ftpserver username*/
            char ftppass[32] = FTPSERVER_PASSWORD;              /** @brief ftpserver password*/
        #endif
        wifictl_networklist* networklist = NULL;                /** @brief network list config pointer */

        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad(JsonDocument& document);
        virtual bool onSave(JsonDocument& document);
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 2000; }
    };

#endif // _WHEELCTLCONFIG_H