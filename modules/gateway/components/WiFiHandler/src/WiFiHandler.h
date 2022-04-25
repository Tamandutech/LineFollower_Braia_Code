#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <atomic>
#include <mutex>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "thread.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class WiFiHandler : public Thread
{
public:
    static WiFiHandler *getInstance(std::string name = "WiFiHandler", uint32_t stackDepth = 10000, UBaseType_t priority = 9)
    {
        WiFiHandler *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new WiFiHandler(name, stackDepth, priority);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    void Run() override;

private:
    std::string name;

    static std::atomic<WiFiHandler *> instance;
    static std::mutex instanceMutex;

    WiFiHandler(std::string name, uint32_t stackDepth, UBaseType_t priority);
};

#endif