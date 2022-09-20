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

#include "cJSON.h"
#include "better_console.hpp"

#include "RobotData.h"

using namespace cpp_freertos;

#include "esp_log.h"

#define SERVICE_UART_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SERVICE_STREAM_UUID "3A8328FB-3768-46D2-B371-B34864CE8025" // Stream service
#define CHARACTERISTIC_STREAM_TX "3A8328FC-3768-46D2-B371-B34864CE8025"

struct ble_gatt_uart_packet_t
{
    uint8_t *payload;           /*!< Pre-allocated data buffer */
    size_t len;                 /*!< Length of the BLE data */
    NimBLEAddress clientMAC;    /*!< Client MAC address */
};

class BLEServerService : public Thread, public Singleton<BLEServerService>
{
public:
    BLEServerService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

    bool deviceConnected = false;
    bool oldDeviceConnected = false;

    ble_gatt_uart_packet_t packetReceived;
    static QueueHandle_t queuePacketsReceived;

    BLECharacteristic *pStreamTxCharacteristic;
    TaskHandle_t xTaskStream;

private:
    BLEServer *pServer = NULL;
    BLECharacteristic *pTxCharacteristic;


    uint8_t txValue = 0;
};

#endif
#endif