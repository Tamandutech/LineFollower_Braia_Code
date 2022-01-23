
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

  // Inicializacao do componente de encapsulamento de dado, definindo nome do robo

  braia->getStatus()->setMapping(false);
  bool mapping = braia->getStatus()->getMapping();
  braia->getStatus()->setState(CAR_IN_LINE);

  //Pulsos para uma revolução de cada encoder (revolução*redução)
  // braia->getSpeed()->setMPR_MotDir(20,30);
  // braia->getSpeed()->setMPR_MotEsq(20,30);

  if (mapping)
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
  carStatusService->Start();

  mappingService = new MappingService("MappingService", braia, 10000, 9);
  // carStatusService->Start();

  motorsService = new MotorsService("MotorsService", braia, 10000, 9);
  motorsService->Start();

  pidService = new PIDService("PIDService", braia, 10000, 9);
  pidService->Start();

  sensorsService = new SensorsService("SensorsService", braia, 10000, 9);
  sensorsService->Start();

  speedService = new SpeedService("SpeedService", braia, 10000, 9);
  speedService->Start();
}