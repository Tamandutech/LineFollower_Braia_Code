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

  ledsService = LEDsService::getInstance("LEDsService", 4096, 9);
  ledsService->Start();

  led_command_t command;
  command.led[0] = LED_POSITION_FRONT;
  command.led[1] = LED_POSITION_NONE;
  command.led[2] = LED_POSITION_NONE;
  command.color = LED_COLOR_RED;
  command.effect = LED_EFFECT_SET;
  command.brightness = 1;

  ESP_LOGD("MAIN", "LED Vermelho");
  ledsService->queueCommand(command);

  carStatusService = CarStatusService::getInstance("CarStatusService",10000,19);
  sensorsService = SensorsService::getInstance("SensorsService", 8192, 20);
  mappingService = MappingService::getInstance("MappingService", 2048, 18);
  motorsService = MotorsService::getInstance("MotorsService", 2048, 20);
  speedService = SpeedService::getInstance("SpeedService", 2048, 20);
  pidService = PIDService::getInstance("PIDService", 4096, 20);
  espNowHandler = ESPNOWHandler::getInstance("ESPNOWHandler", 8192, 9);

  ESP_LOGD("MAIN", "LED Laranja");
  command.color = LED_COLOR_ORANGE;
  ledsService->queueCommand(command);

  sensorsService->Start();
  motorsService->Start();
  pidService->Start();
  speedService->Start();
  carStatusService->Start();
  mappingService->Start();

  ESP_LOGD("MAIN", "LED Magenta");
  command.color = LED_COLOR_MAGENTA;
  ledsService->queueCommand(command);

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  ESP_LOGD("MAIN", "Apagando LEDs");
  command.color = LED_COLOR_BLACK;
  ledsService->queueCommand(command);

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
  for (;;)
   {
     ESP_LOGD("main", "carStatusService: %d", eTaskGetState(carStatusService->GetHandle()));
     ESP_LOGD("main", "mappingService: %d", eTaskGetState(mappingService->GetHandle()));
     ESP_LOGD("main", "motorsService: %d", eTaskGetState(motorsService->GetHandle()));
     ESP_LOGD("main", "pidService: %d", eTaskGetState(pidService->GetHandle()));
     ESP_LOGD("main", "sensorsService: %d", eTaskGetState(sensorsService->GetHandle()));
     ESP_LOGD("main", "speedService: %d", eTaskGetState(speedService->GetHandle()));
     ESP_LOGD("main", "ledsService: %d", eTaskGetState(ledsService->GetHandle()));

     vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
#endif
}

#endif