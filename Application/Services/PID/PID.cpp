/*
 * PID.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "PID.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Drivers/QTRSensorDriver/QTRSensorDriver.hpp"

float PID::lastError = 0;

float PID::getPID() {
  float arrayError = QTRSensorDriver::getError();

  float P   = arrayError;
  float D   = arrayError - lastError;
  float PID = (RobotEnv::kp * P) + (RobotEnv::kd * D);

  lastError = arrayError;

  return PID;
}