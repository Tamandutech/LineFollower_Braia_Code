#ifdef TARGET_GATEWAY

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "SerialService.hpp"
#include "ServerService.hpp"

#include "cmd_system.hpp"
#include "cmd_param.hpp"
#include "cmd_remote.hpp"
#include "cmd_wifi.hpp"

#include "ESPNOWHandler.h"
#include "WifiHandler.h"

SerialService *serialService;
ServerService *serverService;

WiFiHandler *wifiHandler;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

extern "C"
{
    void app_main(void);
}

void app_main(void)
{
    wifiHandler = WiFiHandler::getInstance("WiFiHandler", 10000, 9);
    // wifiHandler->wifi_init_softap("ESP32-AP", "12345678");
    // wifiHandler->wifi_init_sta("BraiaServer", "braiamaster");
    wifiHandler->wifi_init_sta("RFREITAS", "963443530");

    ESPNOWHandler::getInstance("ESPNOWHandler", 10000, 9);

    register_system();
    register_cmd_param();
    register_cmd_remote();
    register_cmd_wifi();

    serialService = SerialService::getInstance("SerialService", 10000, 9);
    serialService->Start();

    serverService = ServerService::getInstance("ServerService", 10000, 9);
    serverService->Start();
}

#endif