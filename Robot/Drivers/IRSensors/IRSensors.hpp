/*
 * IRSensors.hpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_IRSENSORS_IRSENSORS_HPP_
#define DRIVERS_IRSENSORS_IRSENSORS_HPP_

#include "../../Utils/Logger/Logger.hpp"
#include <cstdint>

class IRSensors {
public:
  enum Sensor : uint8_t {
    // Left
    L_1 = 0,
    L_2,

    // Center
    FirstCentral,
    C_1 = FirstCentral,
    C_2,
    C_3,
    C_4,
    C_5,
    C_6,
    C_7,
    C_8,
    C_9,
    C_10,
    C_11,
    C_12,
    LastCentral = C_12,

    // Right
    R_1,
    R_2,

    N_SENSORS_
  };


  static void     calibrateSensors();
  static uint16_t readLine();
  static int16_t  getError();
  static void     setArraySensorCenter(uint16_t center);

protected:
  static uint16_t sensorValues[N_SENSORS_];
  static void     readCalibrated();

private:
  static const volatile uint32_t *const rawSensorValues[N_SENSORS_];
  static uint32_t                       maxValues[N_SENSORS_];
  static uint32_t                       minValues[N_SENSORS_];
  static bool                           calibrated;
  static uint16_t                       lastPosition;
  static uint16_t                       arraySensorCenter;
  static Logger                        *logger;
  const static Sensor                   firstCentralSensor;
  const static Sensor                   lastCentralSensor;
};

#endif /* DRIVERS_IRSENSORS_IRSENSORS_HPP_ */

// Erik Satie: Gymnopédies
