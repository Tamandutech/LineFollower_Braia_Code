
#include "includes.hpp"
#include "RobotData.h"

#include "DebugService.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
//#define LINE_COLOR_BLACK
#define taskStatus false

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Componentes de encapsulamento das variaveis
Robot *braia;

// TaksHandles para gerenciar execucao das tasks
TaskHandle_t xTaskMotors;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskSpeed;
TaskHandle_t xTaskMapping;

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  DebugService debugService("DebugService", 10000, 9);
  debugService.create();

  // // Inicializacao do componente de encapsulamento de dado, definindo nome do robo
  // braia = new Robot("Braia");
  
  // braia->getStatus()->setMapping(false);
  // bool mapping = braia->getStatus()->getMapping();
  // braia->getStatus()->setState(CAR_IN_LINE);

  // //Pulsos para uma revolução de cada encoder (revolução*redução)
  // // braia->getSpeed()->setMPR_MotDir(20,30);
  // // braia->getSpeed()->setMPR_MotEsq(20,30);

  // if(mapping){

  //   braia->getSpeed()->setSpeedBase(30, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedBase(30, CAR_IN_CURVE);

  //   braia->getSpeed()->setSpeedMax(60, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedMax(60, CAR_IN_CURVE);

  //   braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
  //   braia->getPIDVel()->setKd(0.00, CAR_IN_LINE);
  //   braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKd(0.00, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
  //   braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
  //   braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKp(0.5, CAR_IN_LINE);
  //   braia->getPIDVel()->setKp(0.03, CAR_IN_LINE);
  //   braia->getPIDRot()->setKp(0.5, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKp(0.03, CAR_IN_CURVE);

  //   braia->getPIDVel()->setSetpoint(400);

  // }
  // else{

  //   braia->getSpeed()->setSpeedBase(50, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedBase(50, CAR_IN_CURVE);

  //   braia->getSpeed()->setSpeedMax(80, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedMax(60, CAR_IN_CURVE);

  //   braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
  //   braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKd(0.004, CAR_IN_LINE);
  //   braia->getPIDVel()->setKd(0.0, CAR_IN_LINE);
  //   braia->getPIDRot()->setKd(0.004, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKd(0.0, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
  //   braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
  //   braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

  //   braia->getPIDRot()->setKp(0.79, CAR_IN_LINE);
  //   braia->getPIDVel()->setKp(0.055, CAR_IN_LINE);
  //   braia->getPIDRot()->setKp(0.79, CAR_IN_CURVE);
  //   braia->getPIDVel()->setKp(0.055, CAR_IN_CURVE);

  //   braia->getPIDVel()->setSetpoint(1900);

  // }
  
  // Criacao das tasks e definindo seus parametros
  //xTaskCreate(FUNCAO, NOME, TAMANHO DA HEAP, ARGUMENTO, PRIORIDADE, TASK HANDLE)

  // xTaskCreate(vTaskMotors, "TaskMotors", 10000, braia, 9, &xTaskMotors);

  // xTaskCreate(vTaskSensors, "TaskSensors", 10000, braia, 9, &xTaskSensors);

  // xTaskCreate(vTaskPID, "TaskPID", 10000, braia, 9, &xTaskPID);

  // xTaskCreate(vTaskSpeed, "TaskSpeed", 10000, braia, 9, &xTaskSpeed);

  // xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, braia, 9, &xTaskCarStatus);

  // xTaskCreate(vTaskMapping, "TaskMapping", 10000, braia, 9, &xTaskMapping);

}