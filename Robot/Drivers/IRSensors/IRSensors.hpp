/*
 * IRSensors.hpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_IRSENSORS_IRSENSORS_HPP_
#define DRIVERS_IRSENSORS_IRSENSORS_HPP_

#include "../../Context/Definitions.hpp"
#include "../../Utils/Logger/Logger.hpp"

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

  static const uint16_t (&sensorValues)[N_SENSORS_];
  static const uint16_t &position;
  static const int16_t  &error;
  static const bool (&mark)[N_SIDES_];
  static const bool &isOnLine;
  static const bool &isOnCross;

  static void update();
  static void calibrate();

private:
  static void readCalibrated();

  static const volatile uint32_t *const rawValues[N_SENSORS_];
  static uint16_t                       sensorValue_[N_SENSORS_];
  static uint32_t                       markDetecionTime_[N_SIDES_];
  static uint32_t                       maxRawValues_[N_SENSORS_];
  static uint32_t                       minRawValues_[N_SENSORS_];
  static uint16_t                       previousPosition_;
  static uint16_t                       position_;
  static int16_t                        error_;
  static bool                           previousMark_[N_SIDES_];
  static bool                           mark_[N_SIDES_];
  static bool                           isOnLine_;
  static bool                           isOnCross_;
  static bool                           calibrated_;
  static Logger                        *logger;
};

#endif /* DRIVERS_IRSENSORS_IRSENSORS_HPP_ */

// Erik Satie: Gymnopédies
