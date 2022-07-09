#ifndef BLE_SERVER_SERVICE_H
#define BLE_SERVER_SERVICE_H

#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED)

#include "thread.hpp"
#include "singleton.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"

#include "NimBLEDevice.h"
#include "NimBLELog.h"

#include "RobotData.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define SERVICE_UART_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BLEServerService : public Thread, public Singleton<BLEServerService>
{
public:
    BLEServerService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;
    
    bool deviceConnected = false;
    bool oldDeviceConnected = false;

private:
    BLEServer *pServer = NULL;
    BLECharacteristic *pTxCharacteristic;

    uint8_t txValue = 0;

    uint8_t uniqueIdCounter = 0;
    uint8_t GetUniqueID();
};

#endif
#endif