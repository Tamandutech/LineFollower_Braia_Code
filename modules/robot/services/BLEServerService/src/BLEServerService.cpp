#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED)

#include "BLEServerService.hpp"

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client conectado.");
        BLEServerService::getInstance()->deviceConnected = true;
    };
    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Endereço do client: %s", NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 3x interval time for best results.
         */
        BLEServerService::getInstance()->deviceConnected = true;
    };
    void onDisconnect(NimBLEServer *pServer)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client disconnected - start advertising");
        BLEServerService::getInstance()->deviceConnected = false;
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "MTU updated: %u for connection ID: %u", MTU, desc->conn_handle);
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Server Passkey Request");
        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "The passkey YES/NO number: %d", pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            /** NOTE: createServer returns the current server reference unless one is not already created */
            NimBLEDevice::createServer()->disconnect(desc->conn_handle);
            ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Encrypt connection failed - disconnecting client");
            return;
        }
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s : onRead(), value: %s",
                 pCharacteristic->getUUID().toString().c_str(),
                 pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s : onWrite(), value: %s",
                 pCharacteristic->getUUID().toString().c_str(),
                 pCharacteristic->getValue().c_str());
    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Sending notification to clients");
    };

    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Notification/Indication status code: %d , return code: %d, %s",
                 status,
                 code,
                 NimBLEUtils::returnCodeToString(code));
    };
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks
{
    void onWrite(NimBLEDescriptor *pDescriptor)
    {
        std::string dscVal((char *)pDescriptor->getValue(), pDescriptor->getLength());
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Descriptor witten value: %s", dscVal.c_str());
    };

    void onRead(NimBLEDescriptor *pDescriptor)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s Descriptor read", pDescriptor->getUUID().toString().c_str());
    };
    ;
};

/** Define callback instances globally to use for multiple Charateristics \ Descriptors */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

BLEServerService::BLEServerService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    ESP_LOGD(this->GetName().c_str(), "Iniciando servidor GATT...");

    NimBLEDevice::init(Robot::getInstance()->GetName());
    NimBLEDevice::setPower(esp_power_level_t::ESP_PWR_LVL_P9, esp_ble_power_type_t::ESP_BLE_PWR_TYPE_ADV);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UART_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        /******* Enum Type NIMBLE_PROPERTY now *******
            BLECharacteristic::PROPERTY_NOTIFY
            );
        **********************************************/
        NIMBLE_PROPERTY::NOTIFY);

    /***************************************************
     NOTE: DO NOT create a 2902 descriptor
     it will be created automatically if notifications
     or indications are enabled on a characteristic.

     pCharacteristic->addDescriptor(new BLE2902());
    ****************************************************/

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        /******* Enum Type NIMBLE_PROPERTY now *******
                BLECharacteristic::PROPERTY_WRITE
                );
        *********************************************/
        NIMBLE_PROPERTY::WRITE);

    pRxCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // Start the service
    pService->start();

    pServer->getAdvertising()->start();

    ESP_LOGD(this->GetName().c_str(), "Começou a se anunciar...");
}

void BLEServerService::Run()
{
    ESP_LOGD(this->GetName().c_str(), "Iniciando Loop...");
    for (;;)
    {
        if (deviceConnected)
        {
            std::string teste = "Tempo: " + std::to_string(esp_timer_get_time());
            pTxCharacteristic->setValue(teste);
            pTxCharacteristic->notify();
        }

        // disconnecting
        if (!deviceConnected && oldDeviceConnected)
        {
            pServer->startAdvertising(); // restart advertising
            ESP_LOGD(this->GetName().c_str(), "start advertising\n");
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected)
        {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay between loops to reset watchdog timer
    }
}

#endif