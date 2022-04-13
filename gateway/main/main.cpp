/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "SerialService.hpp"

#include "cmd_system.hpp"
#include "cmd_robot.hpp"

#include "EspNowHandler.h"

SerialService *serialService;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

extern "C"
{
    void app_main(void);
}

void app_main(void)
{
    EspNowHandler::getInstance()->EspNowInit(1, broadcastAddress, false);

    register_system();
    register_cmd_robot();

    serialService = new SerialService("SerialService", 10000, 9);
    serialService->Start();
}
