/****************************************************************************
 *  Jesper Ortlund 2021
 *  Based on code Copyright 2020 Dirk Brosswick
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
#ifndef _BLECTL_H
    #define _BLECTL_H

    #include "TTGO.h"
    #include "callback.h"

    #define BLECTL_CONNECT               _BV(0)         /** @brief event mask for blectl connect to an client */
    #define BLECTL_DISCONNECT            _BV(1)         /** @brief event mask for blectl disconnect */
    #define BLECTL_STANDBY               _BV(2)         /** @brief event mask for blectl standby */
    #define BLECTL_ON                    _BV(3)         /** @brief event mask for blectl on */
    #define BLECTL_OFF                   _BV(4)         /** @brief event mask for blectl off */
    #define BLECTL_ACTIVE                _BV(5)         /** @brief event mask for blectl active */
    #define BLECTL_MSG                   _BV(6)         /** @brief event mask for blectl msg */
    #define BLECTL_PIN_AUTH              _BV(7)         /** @brief event mask for blectl for pin auth, callback arg is (uint32*) */
    #define BLECTL_PAIRING               _BV(8)         /** @brief event mask for blectl pairing requested */
    #define BLECTL_PAIRING_SUCCESS       _BV(9)         /** @brief event mask for blectl pairing success */
    #define BLECTL_PAIRING_ABORT         _BV(10)        /** @brief event mask for blectl pairing abort */
    #define BLECTL_MSG_SEND_SUCCESS      _BV(11)        /** @brief event mask msg send success */
    #define BLECTL_MSG_SEND_ABORT        _BV(12)        /** @brief event mask msg send abort */
    #define BLECTL_CLI_DOCONNECT         _BV(13)        /** @brief event mask for ble client connect to a server */
    #define BLECTL_CLI_DOSCAN            _BV(14)        /** @brief event mask for ble client scanning */
    #define BLECTL_CLI_CONNECTED         _BV(15)        /** @brief event mask for ble client connected */
    #define BLECTL_CLI_DISCONNECTED      _BV(16)        /** @brief event mask for ble client disconnected */
    #define BLECTL_CLI_ON                _BV(17)        /** @brief event mask for ble client on */
    #define BLECTL_CLI_OFF               _BV(18)        /** @brief event mask for ble client off */
    #define BLECTL_CLI_MSG               _BV(19)        /** @brief event mask for client blectl msg */
    #define BLECTL_CLI_DETECT            _BV(20)        /** @brief event mask for detecting new wheels */

    // See the following for generating UUIDs:
    // https://www.uuidgenerator.net/
    #define SERVICE_UUID                                    BLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")     /** @brief UART service UUID */
    #define CHARACTERISTIC_UUID_RX                          BLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
    #define CHARACTERISTIC_UUID_TX                          BLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")

    /*
    * Default BLE UUIDs for various EUC mfgs
    */

    #define DESCRIPTOR_UUID                                 BLEUUID("00002902-0000-1000-8000-00805f9b34fb")

    //Default UUIDs for KS wheels
    #define KS_SERVICE_UUID_1                               BLEUUID("0000fff0-0000-1000-8000-00805f9b34fb")
    #define KS_SERVICE_UUID_2                               BLEUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
    #define KS_CHAR_UUID                                    BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb")
    #define KS_WRITE_CHAR_UUID                              BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb") 
    //Default UUIDs for GW wheels -- Not verified
    #define GW_SERVICE_UUID_1                               BLEUUID("0000fff0-0000-1000-8000-00805f9b34fb")
    #define GW_SERVICE_UUID_2                               BLEUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
    #define GW_CHAR_UUID                                    BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb")
    #define GW_WRITE_CHAR_UUID                              BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb")
    //Default UUIDs for IM wheels -- Not verified
    #define IM_SERVICE_UUID                                 BLEUUID("0000fff0-0000-1000-8000-00805f9b34fb")
    #define IM_WRITE_SERVICE_UUID                           BLEUUID("0000ffe5-0000-1000-8000-00805f9b34fb")
    #define IM_READ_CHAR_UUID                               BLEUUID("0000ffe4-0000-1000-8000-00805f9b34fb")
    #define IM_WRITE_CHAR_UUID                              BLEUUID("0000ffe9-0000-1000-8000-00805f9b34fb")
    //Default UUIDs for NB_Z wheels -- Not verified
    #define NBZ_SERVICE_UUID                                 BLEUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
    #define NBZ_READ_CHAR_UUID                               BLEUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e")
    #define NBZ_WRITE_CHAR_UUID                              BLEUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
    //Default UUIDs for older NB wheels -- Not verified
    #define NB_SERVICE_UUID                                 BLEUUID("0000ffe0-0000-1000-8000-00805f9b34fb")
    #define NB_READ_CHAR_UUID                               BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb")
    #define NB_WRITE_CHAR_UUID                              BLEUUID("0000ffe1-0000-1000-8000-00805f9b34fb")
    //End Wheel config

    #define DEVICE_INFORMATION_SERVICE_UUID                 BLEUUID((uint16_t)0x180A)                           /** @brief Device Information server UUID */
    #define MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID    BLEUUID((uint16_t)0x2A29)                           /** @brief Device Information - manufacturer name string UUID */
    #define FIRMWARE_REVISION_STRING_CHARACTERISTIC_UUID    BLEUUID((uint16_t)0x2A26)                           /** @brief Device Information - firmware revision UUID */

    #define BATTERY_SERVICE_UUID                            BLEUUID((uint16_t)0x180F)                           /** @brief Battery service UUID */
    #define BATTERY_LEVEL_CHARACTERISTIC_UUID               BLEUUID((uint16_t)0x2A19)                           /** @brief battery level characteristic UUID */
    #define BATTERY_LEVEL_DESCRIPTOR_UUID                   BLEUUID((uint16_t)0x2901)                           /** @brief battery level descriptor UUID */
    #define BATTERY_POWER_STATE_CHARACTERISTIC_UUID         BLEUUID((uint16_t)0x2A1A)                           /** @brief battery power state characteristic UUID */

    #define BATTERY_POWER_STATE_BATTERY_UNKNOWN             0x0
    #define BATTERY_POWER_STATE_BATTERY_NOT_SUPPORTED       0x1
    #define BATTERY_POWER_STATE_BATTERY_NOT_PRESENT         0x2
    #define BATTERY_POWER_STATE_BATTERY_PRESENT             0x3

    #define BATTERY_POWER_STATE_DISCHARGE_UNKNOWN           0x0
    #define BATTERY_POWER_STATE_DISCHARGE_NOT_SUPPORTED     0x4
    #define BATTERY_POWER_STATE_DISCHARGE_NOT_DISCHARING    0x8
    #define BATTERY_POWER_STATE_DISCHARGE_DISCHARING        0xc

    #define BATTERY_POWER_STATE_CHARGE_UNKNOWN              0x0
    #define BATTERY_POWER_STATE_CHARGE_NOT_CHARGEABLE       0x10
    #define BATTERY_POWER_STATE_CHARGE_NOT_CHARING          0x20
    #define BATTERY_POWER_STATE_CHARGE_CHARING              0x30

    #define BATTERY_POWER_STATE_LEVEL_UNKNOWN               0x0
    #define BATTERY_POWER_STATE_LEVEL_NOT_SUPPORTED         0x40
    #define BATTERY_POWER_STATE_LEVEL_GOOD                  0x80
    #define BATTERY_POWER_STATE_LEVEL_CRITICALLY_LOW        0xC0

    #define BLECTL_JSON_COFIG_FILE         "/blectl.json"   /** @brief defines json config file name */
    #define BLECTL_JSON_WHEEL_FILE         "/blectl_wheel.json"   /** @brief defines json stored wheels file name */

    #define EndofText               0x03
    #define LineFeed                0x0a
    #define DataLinkEscape          0x10

    #define BLECTL_CHUNKSIZE        20      /** @brief chunksize for send msg */
    #define BLECTL_CHUNKDELAY       20      /** @brief chunk delay in ms for each msg chunk */
    #define BLECTL_MSG_MTU          256     /** @brief max msg size */

    #define BLECTL_MAX_ADVERTISED   5      /** @brief max number of wheels to simultaniously show */

    

    /**
     * @brief blectl config structure
     */
    typedef struct {
        bool autoon = false;             /** @brief auto on/off */
        bool enable_on_standby = false; /** @brief enable on standby on/off */
        int32_t txpower = 1;            /** @brief tx power, valide values are from 0 to 4 */
        bool autoconnect = true;      /** @brief enable autoconnect to wheel on/off */
    } blectl_config_t;

    enum { 
        WHEELTYPE_KS,
        WHEELTYPE_GW,
        WHEELTYPE_IM,
        WHEELTYPE_NB,
        WHEELTYPE_NBZ,
        WHEELTYPE_NUM
    };

    typedef struct {
        String address = "00:00:00:00:00:00";
        byte type = WHEELTYPE_NUM;
    } stored_wheel_t;

    enum { 
        WHEEL_1,
        WHEEL_2,
        WHEEL_3,
        WHEEL_4,
        WHEEL_5,
        MAX_STORED_WHEELS
    };

    typedef struct {
        String address = "00:00:00:00:00:00";
        byte type = WHEELTYPE_NUM;
    } detected_wheel_t;

    enum { 
        DETECTED_1,
        DETECTED_2,
        DETECTED_3,
        DETECTED_4,
        DETECTED_5,
        MAX_DETECTED_WHEELS
    };

    /**
     * @brief blectl send msg structure
     */
    typedef struct {
        char *msg;                      /** @brief pointer to an sending msg */
        bool active;                    /** @brief send msg structure active */
        int32_t msglen;                 /** @brief msg lenght */
        int32_t msgpos;                 /** @brief msg postition for next send */
    } blectl_msg_t;

    /**
     * @brief ble setup function
     */
    void blectl_setup( void );
    /**
     * @brief trigger a blectl managemt event
     * 
     * @param   bits    event to trigger
     */
    void blectl_set_event( EventBits_t bits );
    /**
     * @brief clear a blectl managemt event
     * 
     * @param   bits    event to clear
     */
    void blectl_clear_event( EventBits_t bits );
    /**
     * @brief get a blectl managemt event state
     * 
     * @param   bits    event state, example: POWERMGM_STANDBY to evaluate if the system in standby
     */
    bool blectl_get_event( EventBits_t bits );
    /**
     * @brief registers a callback function which is called on a corresponding event
     * 
     * @param   event  possible values:     BLECTL_CONNECT,
     *                                      BLECTL_DISCONNECT,
     *                                      BLECTL_STANDBY,
     *                                      BLECTL_ON,
     *                                      BLECTL_OFF,       
     *                                      BLECTL_ACTIVE,    
     *                                      BLECTL_MSG,
     *                                      BLECTL_PIN_AUTH,
     *                                      BLECTL_PAIRING,
     *                                      BLECTL_PAIRING_SUCCESS,
     *                                      BLECTL_PAIRING_ABORT
     * @param   blectl_event_cb     pointer to the callback function
     * @param   id                  pointer to an string
     */
    bool blectl_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id );
    /**
     * @brief enable blueetooth on standby
     * 
     * @param   enable_on_standby   true means enabled, false means disabled 
     */
    void blectl_set_enable_on_standby( bool enable_on_standby );
    /**
     * @brief enable advertising
     * 
     * @param   advertising true means enabled, false means disabled
     */
    void blectl_set_advertising( bool advertising );

    void blectl_set_autoconnect(bool autoconnect);

    bool blectl_get_autoconnect(void);

    /**
     * @brief get the current enable_on_standby config
     * 
     * @return  true means enabled, false means disabled
     */
    bool blectl_get_enable_on_standby( void );
    /**
     * @brief get the current advertising config
     * 
     * @return  true means enabled, false means disabled
     */
    bool blectl_get_advertising( void );
    /**
     * @brief store the current configuration to SPIFFS
     */
    void blectl_save_config( void );
    /**
     * @brief read the configuration from SPIFFS
     */
    void blectl_read_config( void );
    /**
     * @brief send an battery update over bluetooth to gadgetbridge
     * 
     * @param   percent     battery percent
     * @param   charging    charging state
     * @param   plug        powerplug state
     */
    void blectl_update_battery( int32_t percent, bool charging, bool plug );
    /**
     * @brief send an message over bluettoth to gadgetbridge
     * 
     * @param   msg     pointer to a string
     */
    void blectl_send_msg( char *msg );
    /**
     * @brief set the transmission power
     * 
     * @param   txpower power from 0..4, from -12db to 0db in 3db steps
     */
    void blectl_set_txpower( int32_t txpower );
    /**
     * @brief get the current transmission power
     * 
     * @return  power from 0..4, from -12db to 0db in 3db steps
     */
    int32_t blectl_get_txpower( void );
    /**
     * @brief enable the bluettoth stack
     */
    void blectl_on( void );
    /**
     * @brief disable the bluetooth stack
     */
    void blectl_off( void );
    /**
     * @brief get the current enable config
     * 
     * @return true if bl enabled, false if bl disabled
     */
    bool blectl_get_autoon( void );
    /**
     * @brief set the current bl enable config
     * 
     * @param enable true if enabled, false if disable
     */
    void blectl_set_autoon( bool autoon );

    void writeBLE (byte*, int);
    /**
     * @brief ble client setup function
     */
    void blectl_scan_setup(void);
    /**
     * @brief get BLE connection status
     * 
     * @return true if connected to the whhel, false if disconnected
     */
    bool blectl_cli_getconnected( void );
    /**
     * @brief scan for EUCs via BLE
     * 
     * @param scantime the time scanning is active (1-2sec is usually sufficient)
     * 
     * @param scan_for_new true if scanning for new wheels, false if connecting to stored wheel
     * 
     * @return N/A
     */
    void blectl_scan_once(int scantime, bool scan_for_new);
    /**
     * @brief Save stored wheels to flash
     * 
     * @return true if successful, false otherwise
     */ 
    bool blectl_save_stored_wheels (void);
    /**
     * @brief get the BLE address of a stored wheel
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return String containing the BLE address of the wheel
     */ 
    String blectl_get_stored_wheel_address(byte wheelnum);
    /**
     * @brief get the wheel type of a stored wheel (KS, GW, IM, NB or NBZ)
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return byte containing the assigned type id of the wheel
     * 0 = WHEELTYPE_KS
     * 1 = WHEELTYPE_GW
     * 2 = WHEELTYPE_IM
     * 3 = WHEELTYPE_NB
     * 4 = WHEELTYPE_NBZ
     * 5 = WHEELTYPE_NUM -- also used when wheeltype cannot be determined
     */
    byte blectl_get_stored_wheel_type(byte wheelnum);
    /**
     * @brief get the BLE address of a detected wheel
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return String containing the BLE address of the wheel
     */
    String blectl_get_detected_wheel_address(byte wheelnum);
    /**
     * @brief get the wheel type of a detected wheel (KS, GW, IM, NB or NBZ)
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return byte containing the assigned type id of the wheel
     * 0 = WHEELTYPE_KS,
     * 1 = WHEELTYPE_GW,
     * 2 = WHEELTYPE_IM,
     * 3 = WHEELTYPE_NB,
     * 4 = WHEELTYPE_NBZ,
     * 5 = WHEELTYPE_NUM -- also used when wheeltype cannot be determined
     */
    byte blectl_get_detected_wheel_type(byte wheelnum);
    /**
     * @brief get the maximum number of detected wheels
     *  
     * @return byte containing max number of detected wheels
     */
    byte blectl_get_max_detected_wheels(void);
    /**
     * @brief get the status of a wheel storage slot
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return true if wheel is stored in the slot, false otherwise
     */
    bool blectl_stored_wheel_exist(byte wheelnum);
    /**
     * @brief check if wheel is in storage slot
     * 
     * @param addr the mac-address of the wheel
     * 
     * @return true if wheel with the address is stored, false otherwise
     */
    bool blectl_wheeladdress_stored(String addr);
    /**
     * @brief get the next free wheel storage slot
     *  
     * @return byte containing the first free slot number
     */
    byte blectl_get_free_wheelslot( void );
    /**
     * @brief add a wheel to a storage slot
     * 
     * @param wheeladdress String containing the BLE address of the wheel to be stored
     * @param wheeltype byte containing the assigned type id of the wheel
     * 0 = WHEELTYPE_KS,
     * 1 = WHEELTYPE_GW,
     * 2 = WHEELTYPE_IM,
     * 3 = WHEELTYPE_NB,
     * 4 = WHEELTYPE_NBZ,
     * 5 = WHEELTYPE_NUM -- also used when wheeltype cannot be determined
     * 
     * @param wheelslot the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return true if wheel is stored in the slot, false otherwise
     */
    bool blectl_add_stored_wheel(String wheeladdress, byte wheeltype, byte wheelslot);
    /**
     * @brief remove a stored wheel from flash
     * 
     * @param wheelnum the number of the wheel storage slot should be < MAX_STORED_WHEELS
     * 
     * @return true if remove successful, false otherwise
     */
    bool blectl_remove_stored_wheel(byte wheelnum);
    /**
     * @brief move the selected stored wheel to the first storage slot
     * 
     * @param wheelnum the number of the wheel storage slot, should be < MAX_STORED_WHEELS
     * 
     * @return true if successful, false otherwise
     */
    bool blectl_set_prio_stored_wheel(byte wheelnum);

    byte blectl_get_num_detected_wheels(void);

#endif // _BLECTL_H