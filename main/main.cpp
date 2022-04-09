// Serviços do Robô
#include "CarStatusService.hpp"
#include "MappingService.hpp"
#include "MotorsService.hpp"
#include "PIDService.hpp"
#include "SensorsService.hpp"
#include "SpeedService.hpp"
#include "ESPNOWService.hpp"
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

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

//#define LINE_COLOR_BLACK

// Componentes de encapsulamento das variaveis
Robot *braia;

CarStatusService *carStatusService;
MappingService *mappingService;
MotorsService *motorsService;
PIDService *pidService;
SensorsService *sensorsService;
SpeedService *speedService;
ESPNOWService *espnowService;
LEDsService *ledsService;

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  braia = new Robot("Braia");

  ledsService = new LEDsService("LEDsService", braia, 10000, 9);
  ledsService->Start();

  carStatusService = new CarStatusService("CarStatusService", braia, 10000, 9);
  mappingService = new MappingService("MappingService", braia, 10000, 9);
  motorsService = new MotorsService("MotorsService", braia, 10000, 9);
  speedService = new SpeedService("SpeedService", braia, 10000, 9);
  pidService = new PIDService("PIDService", braia, 10000, 9);
  sensorsService = new SensorsService("SensorsService", braia, 10000, 9);
  espnowService = new ESPNOWService("EspNowService", braia, 10000, 9);

  sensorsService->Start();
  motorsService->Start();
  pidService->Start();
  speedService->Start();
  espnowService->Start();
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