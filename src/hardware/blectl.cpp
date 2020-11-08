/****************************************************************************
 *   2020 Jesper Ortlund
 *   Snippets from My-TTGO-Watch by Dirk Brosswick
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

/*
 *  inspire by https://github.com/bburky/t-watch-2020-project
 *
 */
#include "config.h"
#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "blectl.h"
#include "pmu.h"
#include "powermgm.h"
#include "callback.h"
#include "json_psram_allocator.h"
#include "alloc.h"
#include "alloc.h"
#include "gui/mainbar/fulldash_tile/fulldash_tile.h"
#include "gui/mainbar/simpledash_tile/simpledash_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/mainbar.h"

#include "Kingsong.h"

EventGroupHandle_t blectl_status = NULL;
portMUX_TYPE DRAM_ATTR blectlMux = portMUX_INITIALIZER_UNLOCKED;

blectl_config_t blectl_config;
blectl_msg_t blectl_msg;
callback_t *blectl_callback = NULL;

bool blectl_send_event_cb(EventBits_t event, void *arg);
bool blectl_powermgm_event_cb(EventBits_t event, void *arg);
bool blectl_cli_powermgm_event_cb(EventBits_t event, void *arg);
bool blectl_powermgm_loop_cb(EventBits_t event, void *arg);
bool blectl_cli_powermgm_loop_cb(EventBits_t event, void *arg);
bool blectl_pmu_event_cb(EventBits_t event, void *arg);
void blectl_send_next_msg(char *msg);
void blectl_loop(void);
void blectl_cli_loop(void);
void blectl_scan_once(int scantime);
bool connectToServer(void);

bool clidoConnect = false;
bool cliconnected = false;
bool cli_ondisconnect = false;
int scandelay = 0;

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;
uint8_t txValue = 0;
String EUC_Brand = "KingSong";

static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

BLECharacteristic *pBatteryLevelCharacteristic;
BLECharacteristic *pBatteryPowerStateCharacteristic;

char *gadgetbridge_msg = NULL;
uint32_t gadgetbridge_msg_size = 0;
static uint64_t NextMillis = millis();


class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        Serial.println("onConnect");
    }
    void onDisconnect(BLEClient *pclient)
    {
        cli_ondisconnect = true;
        cliconnected = false;
        Serial.println("onDisconnect -- cliconnected is false");
        return;
    }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());
        // We have found a device, let us now see if it contains the service we are looking for.
        if (EUC_Brand = "KingSong")
        {
            if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(KS_SERVICE_UUID_1))
            {
                BLEDevice::getScan()->stop();
                myDevice = new BLEAdvertisedDevice(advertisedDevice);
                clidoConnect = true;
            } // Found our server
        }
    } // onResult
};    // MyAdvertisedDeviceCallbacks

bool blectl_cli_powermgm_event_cb(EventBits_t event, void *arg)
{
    bool retval = true;

    switch (event)
    {
    case POWERMGM_STANDBY:
        if (cliconnected)
        {
            retval = false;
            log_w("standby blocked by wheel being connected");
        }
        else
        {
            log_i("go standby");
        }
        break;
    case POWERMGM_WAKEUP:
        NextMillis = millis();
        log_i("go wakeup");
        break;
    case POWERMGM_SILENCE_WAKEUP:
        NextMillis = millis();
        log_i("go silence wakeup");
        break;
    }
    return (retval);
}

bool blectl_powermgm_loop_cb(EventBits_t event, void *arg)
{
    blectl_loop();
    return (true);
}

bool blectl_cli_powermgm_loop_cb(EventBits_t event, void *arg)
{
    blectl_cli_loop();
    return (true);
}


void blectl_set_event(EventBits_t bits)
{
    portENTER_CRITICAL(&blectlMux);
    xEventGroupSetBits(blectl_status, bits);
    portEXIT_CRITICAL(&blectlMux);
}

void blectl_clear_event(EventBits_t bits)
{
    portENTER_CRITICAL(&blectlMux);
    xEventGroupClearBits(blectl_status, bits);
    portEXIT_CRITICAL(&blectlMux);
}

bool blectl_get_event(EventBits_t bits)
{
    portENTER_CRITICAL(&blectlMux);
    EventBits_t temp = xEventGroupGetBits(blectl_status) & bits;
    portEXIT_CRITICAL(&blectlMux);
    if (temp)
        return (true);

    return (false);
}

bool blectl_register_cb(EventBits_t event, CALLBACK_FUNC callback_func, const char *id)
{
    if (blectl_callback == NULL)
    {
        blectl_callback = callback_init("blectl");
        if (blectl_callback == NULL)
        {
            log_e("blectl callback alloc failed");
            while (true)
                ;
        }
    }
    return (callback_register(blectl_callback, event, callback_func, id));
}

bool blectl_send_event_cb(EventBits_t event, void *arg)
{
    return (callback_send(blectl_callback, event, arg));
}

void blectl_set_enable_on_standby(bool enable_on_standby)
{
    blectl_config.enable_on_standby = enable_on_standby;
    blectl_save_config();
}

void blectl_on(){

}
void blectl_off(){
    
}

void blectl_set_txpower(int32_t txpower)
{
    if (txpower >= 0 && txpower <= 4)
    {
        blectl_config.txpower = txpower;
    }
    switch (blectl_config.txpower)
    {
    case 0:
        BLEDevice::setPower(ESP_PWR_LVL_N12);
        break;
    case 1:
        BLEDevice::setPower(ESP_PWR_LVL_N9);
        break;
    case 2:
        BLEDevice::setPower(ESP_PWR_LVL_N6);
        break;
    case 3:
        BLEDevice::setPower(ESP_PWR_LVL_N3);
        break;
    case 4:
        BLEDevice::setPower(ESP_PWR_LVL_N0);
        break;
    default:
        BLEDevice::setPower(ESP_PWR_LVL_N9);
        break;
    }
    blectl_save_config();
}

void blectl_set_autoon(bool autoon)
{
    blectl_config.autoon = autoon;

    if (autoon)
    {
        blectl_on();
    }
    else
    {
        blectl_off();
    }
    blectl_save_config();
}

int32_t blectl_get_txpower(void)
{
    return (blectl_config.txpower);
}

bool blectl_get_enable_on_standby(void)
{
    return (blectl_config.enable_on_standby);
}

bool blectl_get_autoon(void)
{
    return (blectl_config.autoon);
}

void blectl_save_config(void)
{
    fs::File file = SPIFFS.open(BLECTL_JSON_COFIG_FILE, FILE_WRITE);

    if (!file)
    {
        log_e("Can't open file: %s!", BLECTL_JSON_COFIG_FILE);
    }
    else
    {
        SpiRamJsonDocument doc(1000);

        doc["autoon"] = blectl_config.autoon;
        doc["enable_on_standby"] = blectl_config.enable_on_standby;
        doc["tx_power"] = blectl_config.txpower;
        doc["wheel_mac"] = blectl_config.wheelmac;

        if (serializeJsonPretty(doc, file) == 0)
        {
            log_e("Failed to write config file");
        }
        doc.clear();
    }
    file.close();
}

void blectl_read_config(void)
{
    fs::File file = SPIFFS.open(BLECTL_JSON_COFIG_FILE, FILE_READ);

    if (!file)
    {
        log_e("Can't open file: %s!", BLECTL_JSON_COFIG_FILE);
    }
    else
    {
        int filesize = file.size();
        SpiRamJsonDocument doc(filesize * 2);

        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            log_e("blectl deserializeJson() failed: %s", error.c_str());
        }
        else
        {
            blectl_config.autoon = doc["autoon"] | true;
            blectl_config.enable_on_standby = doc["enable_on_standby"] | false;
            blectl_config.txpower = doc["tx_power"] | 1;
            blectl_config.wheelmac = doc["wheel_mac"] | "NULL";
        }
        doc.clear();
    }
    file.close();
}


static void scanCompleteCB(BLEScanResults scanResults) {
	Serial.printf("Scan complete!\n");
	Serial.printf("We found %d devices\n", scanResults.getCount());
	scanResults.dump();
} // scanCompleteCB

void blectl_cli_loop(void)
{
    int scandelay = 15000;
    if (clidoConnect)
    {
        if (connectToServer())
        {
            Serial.println("We are now connected to the BLE Server.");
            if (EUC_Brand = "Kingsong")
            {
                Serial.println("initialising KingSong");
                initks();
                fulldash_tile_reload();
                simpledash_tile_reload();
                mainbar_jump_to_maintile(LV_ANIM_OFF);
            }
        }
        else
        {
            Serial.println("We have failed to connect to the server;");
        }
        clidoConnect = false;
    }
    if (millis() - NextMillis > scandelay)
    {
        NextMillis += scandelay;
        if (!cliconnected)
        {
            Serial.println("Disconnected... starting scan");
            BLEDevice::getScan()->start(2, scanCompleteCB);
        }
    }
}

void writeBLE(byte *wBLEbyte, int alength)
{
    pRemoteCharacteristic->writeValue(wBLEbyte, alength);
}

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    //Only decode if package contains relevant data
    if (length == 20)
    {
        if (pData[0] == 0xAA && pData[1] == 0x55)
        {
            decodeKS(pData); // For Kingsong only atm.                         
        }
    }
}

bool connectToServer()
{
    cli_ondisconnect = false;
    Serial.print("Forming a connection to ");

    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    delay(50);
    if (cli_ondisconnect){
        Serial.println("connection lost unexpectedly");
        return false;
    }
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(KS_SERVICE_UUID_2);
    if (cli_ondisconnect){
        Serial.println("connection lost unexpectedly - 2");
        return false;
    }

    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(KS_SERVICE_UUID_2.toString().c_str());
        pClient->disconnect();
        cliconnected = false;
        return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(KS_CHAR_UUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(KS_CHAR_UUID.toString().c_str());
        pClient->disconnect();
        cliconnected = false;
        return false;
    }
    Serial.println(" - Found our characteristic");
    // Read the value of the characteristic.
    if (pRemoteCharacteristic->canRead())
    {
        std::string value = pRemoteCharacteristic->readValue();
    }
    // Register for notify
    if (pRemoteCharacteristic->canNotify() == true)
    {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }
    cliconnected = true;
    return cliconnected;
}

bool blectl_cli_getconnected( void )
{ 
    return cliconnected;
}

void blectl_scan_once(int scantime)
{ 
    BLEDevice::getScan()->start(scantime);
}

void blectl_scan_setup()
{
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");
    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 2 seconds.
    powermgm_register_cb(POWERMGM_SILENCE_WAKEUP | POWERMGM_STANDBY | POWERMGM_WAKEUP, blectl_cli_powermgm_event_cb, "blectl_cli");
    powermgm_register_loop_cb(POWERMGM_SILENCE_WAKEUP | POWERMGM_STANDBY | POWERMGM_WAKEUP, blectl_cli_powermgm_loop_cb, "blectl_cli loop");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(2, scanCompleteCB, false);
}