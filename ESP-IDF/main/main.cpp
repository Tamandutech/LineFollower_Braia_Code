
#include "includes.hpp"
#include "RobotData.h"

#include "CarStatusService.hpp"
#include "MappingService.hpp"
#include "MotorsService.hpp"
#include "PIDService.hpp"
#include "SensorsService.hpp"
#include "SpeedService.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
//#define LINE_COLOR_BLACK
#define taskStatus false

// Componentes de encapsulamento das variaveis
Robot *braia;

CarStatusService *carStatusService;
MappingService *mappingService;
MotorsService *motorsService;
PIDService *pidService;
SensorsService *sensorsService;
SpeedService *speedService;

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  braia = new Robot("Braia");

  if (braia->getStatus()->getMapping())
  {

    braia->getSpeed()->setSpeedBase(30, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(30, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(60, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(60, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.5, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.03, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.5, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.03, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(400);
  }
  else
  {

    braia->getSpeed()->setSpeedBase(50, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(50, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(80, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(60, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.004, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.004, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.79, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.055, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.79, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.055, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(1900);
  }

  carStatusService = new CarStatusService("CarStatusService", braia, 10000, 9);
  mappingService = new MappingService("MappingService", braia, 10000, 9);
  motorsService = new MotorsService("MotorsService", braia, 10000, 9);
  speedService = new SpeedService("SpeedService", braia, 10000, 9);
  pidService = new PIDService("PIDService", braia, 10000, 9);
  sensorsService = new SensorsService("SensorsService", braia, 10000, 9);

  sensorsService->Start();
  pidService->Start();
  speedService->Start();
  motorsService->Start();
  mappingService->Start();
  carStatusService->Start();

  // for (;;)
  // {
  //   ESP_LOGD("main", "carStatusService: %d", eTaskGetState(carStatusService->GetHandle()));
  //   ESP_LOGD("main", "mappingService: %d", eTaskGetState(mappingService->GetHandle()));
  //   ESP_LOGD("main", "motorsService: %d", eTaskGetState(motorsService->GetHandle()));
  //   ESP_LOGD("main", "pidService: %d", eTaskGetState(pidService->GetHandle()));
  //   ESP_LOGD("main", "sensorsService: %d", eTaskGetState(sensorsService->GetHandle()));
  //   ESP_LOGD("main", "speedService: %d", eTaskGetState(speedService->GetHandle()));

  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }
}