/*
 * MotorDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#include <algorithm>

#include "MotorDriver.hpp"
#include "main.h"
#include "tim.h"

#define MAX_DUTY ((int16_t) 1000)

const MotorDriver::Pin MotorDriver::motorPins[_N_MOTORS] = 
{
  // Left
  {
    motor2dir_GPIO_Port,
    motor2dir_Pin,
    &htim8,
    TIM_CHANNEL_1
  },

  // Right
  {
    motor1dir_GPIO_Port,
    motor1dir_Pin,
    &htim8,
    TIM_CHANNEL_3
  }
};


MotorDriver::MotorDriver(Motors newMotor) {
  if (newMotor >= _N_MOTORS) {
    // TODO emit error
    newMotor = Left;
  }

  motor = newMotor;
  HAL_GPIO_WritePin(motorPins[motor].dirPort, motorPins[motor].dirPin, GPIO_PIN_RESET);
  HAL_TIM_PWM_Start(motorPins[motor].pwmhtim, motorPins[motor].pwmChannel);

  // TODO EncoderDriver
  // HAL_TIM_Encoder_Start(motorPins[motor].encoderhtim, TIM_CHANNEL_ALL);
} 

void MotorDriver::pwmOutput(int16_t duty) {
  if (duty > 0) {
    // Defines the direction
    HAL_GPIO_WritePin(motorPins[motor].dirPort, motorPins[motor].dirPin, GPIO_PIN_SET);
  } else {
    // Inverts the direction
    HAL_GPIO_WritePin(motorPins[motor].dirPort, motorPins[motor].dirPin, GPIO_PIN_RESET);
    // Makes the duty positive
    duty = -duty;
  }
  // Make sure the duty is within the allowed interval
  duty = std::min(duty, MAX_DUTY);
  __HAL_TIM_SET_COMPARE(motorPins[motor].pwmhtim, motorPins[motor].pwmChannel, duty);
}
