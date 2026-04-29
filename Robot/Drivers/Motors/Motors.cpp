/*
 * MotorDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#include "Motors.hpp"

#include <algorithm>
#include <cstdint>

#include "tim.h"
#include "../../../Robot/Context/RobotEnv.hpp"
#include "../../../Robot/Services/PID/PID.hpp"

const Motors::Pin Motors::motorPins[_N_MOTORS] = {
    // Left
    {motor2dir_GPIO_Port, motor2dir_Pin, &htim8, TIM_CHANNEL_1},

    // Right
    {motor1dir_GPIO_Port, motor1dir_Pin, &htim8, TIM_CHANNEL_3}
};
float   Motors::motorSpeed[_N_MOTORS];
int16_t Motors::motorPWM[_N_MOTORS];

void Motors::pwmOutputFor(Motor motor, int16_t duty) {
  if(duty >= 0) {
    // Defines the direction
    HAL_GPIO_WritePin(motorPins[motor].dirPort, motorPins[motor].dirPin,
                      GPIO_PIN_RESET);
  } else {
    // Inverts the direction
    HAL_GPIO_WritePin(motorPins[motor].dirPort, motorPins[motor].dirPin,
                      GPIO_PIN_SET);
    // Makes the duty positive
    duty = -duty;
  }

  // Make sure the duty is within the allowed interval
  duty            = std::min(duty, RobotEnv::MOTOR_MAX_PWM);
  motorPWM[motor] = duty;
  __HAL_TIM_SET_COMPARE(motorPins[motor].pwmhtim, motorPins[motor].pwmChannel,
                        duty);
}

void Motors::pwmOutput(float desiredSpeed) {
  int u = PID::getPID();

  // The "u" refers to a rotational PID that corrects robot position, so we add
  // an subtract from left and right motors, respectively
  motorSpeed[Left]  = desiredSpeed + u;
  motorSpeed[Right] = desiredSpeed - u;

  pwmOutputFor(Right, motorSpeed[Right]);
  pwmOutputFor(Left, motorSpeed[Left]);
}

void Motors::stop() {
  pwmOutputFor(Left, 0);
  pwmOutputFor(Right, 0);
}
