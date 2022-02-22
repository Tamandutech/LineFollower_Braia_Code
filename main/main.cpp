
#include "includes.hpp"
#include "RobotData.h"
#include "EspNowHandler.h"

#include "CarStatusService.hpp"
#include "MappingService.hpp"
#include "MotorsService.hpp"
#include "PIDService.hpp"
#include "SensorsService.hpp"
#include "SpeedService.hpp"
#include "ESPNOWService.hpp"
#include "LEDsService.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
//#define LINE_COLOR_BLACK
#define taskStatus false // Variável para habilitar a TaskStatus

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

  if (braia->getStatus()->getMapping())
  {

    braia->getSpeed()->setSpeedBase(25, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(25, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(50, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.000, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.000, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.27, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(100);
  }
  else
  {

    braia->getSpeed()->setSpeedBase(40, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(20, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(70, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.27, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(1500);
  }

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

  if (taskStatus)
    carStatusService->Start();

  if (braia->getStatus()->getMapping())
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