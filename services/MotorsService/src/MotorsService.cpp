#include "MotorsService.hpp"

MotorsService::MotorsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
  this->robot = robot;
  this->speed = robot->getSpeed();
  this->status = robot->getStatus();

  // GPIOs dos motores
  motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1, DRIVER_BIN2, DRIVER_PWMB);
  motors.setSTBY(DRIVER_STBY);
};

void MotorsService::Run()
{
  // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();
  //((xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS) < 18000
  //TickType_t xInicialTicks = xTaskGetTickCount();
  // Loop
  for (;;)
  {
    if (status->robotState->getData() != CAR_STOPPED) // verificar se o carrinho deveria se mover
    {
      motors.motorForward(0);                                         // motor 0 ligado para frente
      motors.motorForward(1);                                         // motor 1 ligado para frente
      motors.motorSpeed(0, speed->getSpeedLeft(status->robotState->getData()));  // velocidade do motor 0
      motors.motorSpeed(1, speed->getSpeedRight(status->robotState->getData())); // velocidade do motor 1
    }
    else
    {
      motors.motorsStop(); // parar motores
      //ESP_LOGE("EncData: ", "%d", mediaEnc);
      //vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS); //10ms
  }
}