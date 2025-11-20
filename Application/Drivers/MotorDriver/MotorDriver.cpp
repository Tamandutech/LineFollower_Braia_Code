/*
 * MotorDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#include <algorithm>
#include <cstdint>

#include "MotorDriver.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Services/PID/PID.hpp"
#include "tim.h"

const MotorDriver::Pin MotorDriver::motorPins[_N_MOTORS] = {
    // Left
    {motor2dir_GPIO_Port, motor2dir_Pin, &htim8, TIM_CHANNEL_1},

    // Right
    {motor1dir_GPIO_Port, motor1dir_Pin, &htim8, TIM_CHANNEL_3}
};
float   MotorDriver::motorSpeed[_N_MOTORS];
int16_t MotorDriver::motorPWM[_N_MOTORS];

void MotorDriver::pwmOutputFor(Motors motor, int16_t duty) {
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
  duty            = std::min(duty, RobotEnv::MAX_MOTOR_PWM);
  motorPWM[motor] = duty;
  __HAL_TIM_SET_COMPARE(motorPins[motor].pwmhtim, motorPins[motor].pwmChannel,
                        duty);
}

// TODO desiredSpeed should be in m/s (?), but currently its a PWM value
void MotorDriver::pwmOutput(float desiredSpeed) {
  // TODO
  // caso o valor desejado seja maior que a tensão da bateria, o valor
  // desejado é a tensão da bateria
  //    float targetVoltage = desiredVoltage;
  //    if (targetVoltage > get_battery_voltage(adc_buffer)) {
  //        targetVoltage = get_battery_voltage(adc_buffer);
  //    }

  // map a value from 0v to vBat to 0 to 1000
  // float desiredSpeed = (targetVoltage * 1000.0f) /
  // get_battery_voltage(adc_buffer);

  int PID = PID::getPID();

  // TODO why (+ PID) and (- PID)?
  motorSpeed[Left]  = desiredSpeed + PID;
  motorSpeed[Right] = desiredSpeed - PID;

  pwmOutputFor(Right, motorSpeed[Right]);
  pwmOutputFor(Left, motorSpeed[Left]);
}

void MotorDriver::stop() {
  pwmOutputFor(Left, 0);
  pwmOutputFor(Right, 0);
}