#include "MotorsService.h"

void MotorsService::Main(){
  // static const char *TAG = "vTaskMotors";
  // //Robot *braia = (Robot *)pvParameters;
  // // Setup
  // ESP_LOGD(TAG, "Task criada!");

  // Robot *braia = (Robot *)pvParameters;
  // dataSpeed *speed = braia->getSpeed();
  // RobotStatus *status = braia->getStatus();

  // // Componente de controle dos motores
  // ESP32MotorControl motors;

  // // GPIOs dos motores
  // motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1, DRIVER_BIN2, DRIVER_PWMB);
  // motors.setSTBY(DRIVER_STBY);

  // // Pausa da Task
  // //vTaskSuspend(xTaskMotors);

  // ESP_LOGD(TAG, "Retomada!");

  // // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  // TickType_t xLastWakeTime = xTaskGetTickCount();
  // //((xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS) < 18000
  // //TickType_t xInicialTicks = xTaskGetTickCount();
  // // Loop
  // for (;;)
  // { 
  //   int32_t mediaEnc = ((speed->getEncRight()) + (speed->getEncLeft())) / 2;
  //   // if(mediaEnc > 3660 && mediaEnc < 4800) {
  //   //   braia->getPIDVel()->setSetpoint(2000);
  //   //   braia->getPIDVel()->setKp(0.07, CAR_IN_LINE);
  //   //   braia->getPIDVel()->setKp(0.07, CAR_IN_CURVE);
  //   // }
  //   // else{
  //   //   braia->getPIDVel()->setSetpoint(1700);
  //   //   braia->getPIDVel()->setKp(0.065, CAR_IN_LINE);
  //   //   braia->getPIDVel()->setKp(0.065, CAR_IN_CURVE);
  //   // }
  //   if (status->getState() != CAR_STOPPED && mediaEnc < 26600) // verificar se o carrinho deveria se mover
  //   {
  //     motors.motorForward(0);                                         // motor 0 ligado para frente
  //     motors.motorForward(1);                                         // motor 1 ligado para frente
  //     motors.motorSpeed(0, speed->getSpeedLeft(status->getState()));  // velocidade do motor 0
  //     motors.motorSpeed(1, speed->getSpeedRight(status->getState())); // velocidade do motor 1
  //   }
  //   else
  //   {
  //     motors.motorsStop(); // parar motores
  //     //ESP_LOGE("EncData: ", "%d", mediaEnc);
  //     //vTaskDelay(1000/ portTICK_PERIOD_MS);
  //   }
  //   vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS); //10ms
  // }
}