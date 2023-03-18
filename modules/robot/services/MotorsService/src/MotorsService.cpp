#include "MotorsService.hpp"

MotorsService::MotorsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
  this->robot = Robot::getInstance();
  this->speed = robot->getSpeed();
  this->status = robot->getStatus();

  // GPIOs dos motores
  motors.attachMotors(DRIVER_AIN2, DRIVER_AIN1, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
  //motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
  motors.setSTBY(DRIVER_STBY);
};

void MotorsService::Run()
{
  // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);

    state = (CarState)status->robotState->getData();

    // if (iloop >= 200 && !status->robotIsMapping->getData())
    // {
    //     iloop = 0;
    //     ESP_LOGD("MotorsService", "State: %d", state);
    // }
    // iloop++;

    if (state != CAR_STOPPED) // verificar se o carrinho deveria se mover
    {
      // motors.motorForward(0);                        // motor 0 ligado para frente
      // motors.motorForward(1);                        // motor 1 ligado para frente
      motors.motorSpeed(0, speed->left->getData());  // velocidade do motor 0
      motors.motorSpeed(1, speed->right->getData()); // velocidade do motor 1
    }
    else
    {
      motors.motorsStop();
    }
  }
}