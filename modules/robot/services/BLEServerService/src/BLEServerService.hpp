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

class BLEServerService : public Thread, public Singleton<BLEServerService>
{
public:
    BLEServerService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    static NimBLEServer *pServer;
};

#endif
#endif