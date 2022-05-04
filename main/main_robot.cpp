#ifdef TARGET_ROBOT

// Serviços do Robô
#include "CarStatusService.hpp"
#include "MappingService.hpp"
#include "MotorsService.hpp"
#include "PIDService.hpp"
#include "SensorsService.hpp"
#include "SpeedService.hpp"
#include "ESPNOWHandler.h"
#include "LEDsService.hpp"

// C/C++
#include <stdbool.h>
#include <string>

// Espressif (ESP-IDF)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"

// Data Objects
#include "RobotData.h"
#include "DataAbstract.hpp"

#include "cmd_system.hpp"
#include "cmd_param.hpp"
#include "better_console.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

// Componentes de encapsulamento das variaveis
Robot *braia;

CarStatusService *carStatusService;
MappingService *mappingService;
MotorsService *motorsService;
PIDService *pidService;
SensorsService *sensorsService;
SpeedService *speedService;
LEDsService *ledsService;
ESPNOWHandler *espNowHandler;

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  better_console_config_t console_config;

  console_config.max_cmdline_args = 8;
  console_config.max_cmdline_length = 256;

  ESP_ERROR_CHECK(better_console_init(&console_config));

  register_system();
  register_cmd_param();

  braia = Robot::getInstance("Braia");

  sensorsService = new SensorsService("SensorsService", braia, 8192, 20);
  carStatusService = CarStatusService::getInstance();
  mappingService = MappingService::getInstance("MappingService", 2048, 18);
  motorsService = new MotorsService("MotorsService", braia, 2048, 20);
  speedService = new SpeedService("SpeedService", braia, 2048, 20);
  pidService = new PIDService("PIDService", braia, 4096, 20);
  espNowHandler = ESPNOWHandler::getInstance("ESPNOWHandler", 8192, 9);

  ledsService = new LEDsService("LEDsService", braia, 2048, 9);
  // ledsService->Start();

  sensorsService->Start();
  motorsService->Start();
  pidService->Start();
  speedService->Start();
  carStatusService->Start();
  mappingService->Start();

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
  for (;;)
  {
    ESP_LOGD("main", "carStatusService: %d", eTaskGetState(carStatusService->GetHandle()));
    ESP_LOGD("main", "mappingService: %d", eTaskGetState(mappingService->GetHandle()));
    ESP_LOGD("main", "motorsService: %d", eTaskGetState(motorsService->GetHandle()));
    ESP_LOGD("main", "pidService: %d", eTaskGetState(pidService->GetHandle()));
    ESP_LOGD("main", "sensorsService: %d", eTaskGetState(sensorsService->GetHandle()));
    ESP_LOGD("main", "speedService: %d", eTaskGetState(speedService->GetHandle()));

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
#endif
}

#endif