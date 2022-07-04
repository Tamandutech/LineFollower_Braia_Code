#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"


using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

class BLEService : public Thread, public Singleton<BLEService>
{
public:
    
    BLEService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:

};

#endif