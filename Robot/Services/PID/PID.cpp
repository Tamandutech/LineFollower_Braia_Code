/*
 * PID.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "../../../Robot/Services/PID/PID.hpp"

#include "../../../Robot/Context/RobotEnv.hpp"
#include "../../Drivers/IRSensors/IRSensors.hpp"

float PID::lastError = 0;

float PID::getPID() {
  float arrayError = IRSensors::getError();

  float P   = arrayError;
  float D   = arrayError - lastError;
  float PID = (RobotEnv::kp * P) + (RobotEnv::kd * D);

  lastError = arrayError;

  return PID;
}