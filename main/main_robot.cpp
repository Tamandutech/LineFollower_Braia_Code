// Serviços do Robô
#include "CarStatusService.hpp"
#include "MappingService.hpp"
#include "PIDService.hpp"
#include "SensorsService.hpp"
#include "SpeedService.hpp"
#include "LEDsService.hpp"
#include "BLEServerService.hpp"

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
#include "cmd_datamanager.hpp"
#include "cmd_runtime.hpp"
#include "better_console.hpp"

#include "esp_log.h"

// Componentes de encapsulamento das variaveis
Robot *braia;

CarStatusService *carStatusService;
MappingService *mappingService;
PIDService *pidService;
SensorsService *sensorsService;
SpeedService *speedService;
LEDsService *ledsService;
BLEServerService *bleServerService;

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  ledsService = LEDsService::getInstance("LEDsService", 4096, 9);
  ledsService->Start();

  ledsService->LedComandSend(LED_POSITION_FRONT, LED_COLOR_RED, 1);

  ESP_LOGD("Main", "Configurando LOGs...");
  esp_log_level_set("*", ESP_LOG_ERROR);
  //esp_log_level_set("LEDsService", ESP_LOG_ERROR);
  //esp_log_level_set("CarStatusService", ESP_LOG_ERROR);
  esp_log_level_set("BLEServerService", ESP_LOG_DEBUG);
  esp_log_level_set("Main", ESP_LOG_DEBUG);
  //esp_log_level_set("TaskStream", ESP_LOG_ERROR);
  //esp_log_level_set("SensorsService", ESP_LOG_ERROR);
  //esp_log_level_set("SpeedService", ESP_LOG_DEBUG);
  //esp_log_level_set("PIDService", ESP_LOG_ERROR);
  //esp_log_level_set("CMD_PARAM", ESP_LOG_ERROR);
  //esp_log_level_set("DataManager", ESP_LOG_ERROR);
  
  braia = Robot::getInstance("TT_LF_BRAIA_V3");
  ESP_LOGD("Main", "Instanciando Robô...");

  ESP_LOGD("Main", "Configurando Comandos...");
  better_console_config_t console_config;
  console_config.max_cmdline_args = 8;
  console_config.max_cmdline_length = 256;
  ESP_ERROR_CHECK(better_console_init(&console_config));
  ESP_LOGD("Main", "Registrando Comandos...");
  register_system(braia->getADC_handle());
  register_cmd_param();
  register_cmd_datamanager();
  register_cmd_runtime();


  ESP_LOGD("Main", "Configurando Serviços...");
  bleServerService = BLEServerService::getInstance("BLEServerService", 4096, 7);
  bleServerService->Start();
  mappingService = MappingService::getInstance("MappingService", 8192, 8);
  carStatusService = CarStatusService::getInstance("CarStatusService", 10000, 8);
  sensorsService = SensorsService::getInstance("SensorsService", 8192, 9);
  speedService = SpeedService::getInstance("SpeedService", 4096, 9);
  pidService = PIDService::getInstance("PIDService", 4096, 10);


  sensorsService->Start();
  pidService->Start();
  speedService->Start();
  carStatusService->Start();
  mappingService->Start();

  ledsService->LedComandSend(LED_POSITION_FRONT, LED_COLOR_PURPLE, 1);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  ESP_LOGD("Main", "Apagando LEDs");
  ledsService->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);;

  ESP_LOGD(MappingService::getInstance()->GetName().c_str(), "Mapeamento");
  ESP_LOGD(CarStatusService::getInstance()->GetName().c_str(), "CarStatusService");
  ESP_LOGD(SpeedService::getInstance()->GetName().c_str(), "SpeedService");
  ESP_LOGD(PIDService::getInstance()->GetName().c_str(), "PIDService");
  ESP_LOGD(LEDsService::getInstance()->GetName().c_str(), "LEDsService");

  //   for (;;)
  //   {
  //     ESP_LOGD("main", "carStatusService: %d", eTaskGetState(carStatusService->GetHandle()));
  //     ESP_LOGD("main", "mappingService: %d", eTaskGetState(mappingService->GetHandle()));
  //     ESP_LOGD("main", "motorsService: %d", eTaskGetState(motorsService->GetHandle()));
  //     ESP_LOGD("main", "pidService: %d", eTaskGetState(pidService->GetHandle()));
  //     ESP_LOGD("main", "sensorsService: %d", eTaskGetState(sensorsService->GetHandle()));
  //     ESP_LOGD("main", "speedService: %d", eTaskGetState(speedService->GetHandle()));
  //     ESP_LOGD("main", "ledsService: %d", eTaskGetState(ledsService->GetHandle()));

  //     vTaskDelay(1000 / portTICK_PERIOD_MS);
  //   }
}