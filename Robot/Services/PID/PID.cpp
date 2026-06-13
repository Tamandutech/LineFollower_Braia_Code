/*
 * PID.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "../../Services/PID/PID.hpp"

#include "../../Context/RobotEnv.hpp"
#include "../../Drivers/IRSensors/IRSensors.hpp"

float PID::lastError = 0;

float PID::evaluate(const int16_t irSensorError) {
  float P   = irSensorError;
  float D   = irSensorError - lastError;
  float PID = (kp * P) + (kd * D);

  lastError = irSensorError;

  return PID;
}