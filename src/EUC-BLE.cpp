#include "twatch_config.h"
#include "EUCDash.h"
#include "BLEDevice.h"

boolean doConnect = false;
boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

/************************************************
   For Kingsong and Gotway EUCs
   Inmotion/Solowheel and Ninebot/Segway use
   different UUIDs, I do not have any of those
   wheels so cannot test, Might need a different
   init procedure as well. Edit this if you want
   to support additional manufacturers. It would
   be possible to automate this as well.
 *************************************************/
/*
   For some reason the KS wheels announce 2 UUIDs, the first one
   "FFF0" does not contain the data we are looking for
*/

static BLEUUID serviceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
//The Service UUID that contains the useful notifications
static BLEUUID serviceUUID2("0000ffe0-0000-1000-8000-00805f9b34fb");
// The characteristic UUID of all wheel notifications
static BLEUUID charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");

void writeBLE(byte *wBLEbyte, int alength)
{
    pRemoteCharacteristic->writeValue(wBLEbyte, alength);
}

/*******************************************************************
   BLE Callback notify, will loop here when wheel is connected
   Decoding is only done if the data format is correct
   This might need to be changed if adapted to other manufacturers
 ******************************************************************/
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

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
    }
    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        Serial.println("onDisconnect");
    }
};

bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID2);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
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
    connected = true;
    return connected;
}

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
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
        {

            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;

        } // Found our server
    }     // onResult
};        // MyAdvertisedDeviceCallbacks

void BLE_setup()
{
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");
    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(2, false);
}