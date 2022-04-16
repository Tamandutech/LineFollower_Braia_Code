#ifdef TARGET_GATEWAY

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "SerialService.hpp"

#include "cmd_system.hpp"
#include "cmd_param.hpp"
#include "cmd_remote.hpp"

#include "ESPNOWHandler.h"

SerialService *serialService;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

extern "C"
{
    void app_main(void);
}

void app_main(void)
{
    ESPNOWHandler::getInstance();

    register_system();
    register_cmd_param();
    register_cmd_remote();

    serialService = new SerialService("SerialService", 10000, 9);
    serialService->Start();
}

#endif