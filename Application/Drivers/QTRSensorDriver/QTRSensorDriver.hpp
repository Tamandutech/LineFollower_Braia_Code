/*
 * QTRSensorDriver.hpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_QTRSENSORDRIVER_QTRSENSORDRIVER_HPP_
#define DRIVERS_QTRSENSORDRIVER_QTRSENSORDRIVER_HPP_

#include "../../Utils/Logger.hpp"

#define ADC_EXPOSE_SENSORS
#include "adc.h"
#undef ADC_EXPOSE_SENSORS

class QTRSensorDriver {
public:
  typedef _Sensor Sensor;

  static uint16_t sensorValues[_N_SENSORS];

  static void     calibrateSensors();
  static uint16_t readLine();

private:
  static uint32_t     maxValues[_N_SENSORS];
  static uint32_t     minValues[_N_SENSORS];
  static bool         calibrated;
  static uint16_t     lastPosition;
  static Logger      *logger;
  const static Sensor firstCentralSensor;
  const static Sensor lastCentralSensor;

  static void readCalibrated();
};

#endif /* DRIVERS_QTRSENSORDRIVER_QTRSENSORDRIVER_HPP_ */
