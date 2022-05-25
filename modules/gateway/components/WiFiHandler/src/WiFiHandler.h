#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <atomic>
#include <mutex>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "thread.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

class WiFiHandler : public Thread<WiFiHandler>
{
public:

    WiFiHandler(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

    void wifi_init_sta();
    void wifi_init_sta(std::string ssid, std::string password);

    void wifi_init_softap(std::string ssid, std::string password);

private:
    std::string name;
    
    bool wifiAlreadyInit = false;

    /* FreeRTOS event group to signal when we are connected*/
    static EventGroupHandle_t s_wifi_event_group;

    static int s_retry_num;

    static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
    static void event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data);

};

#endif