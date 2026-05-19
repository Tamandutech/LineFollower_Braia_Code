/*
 * IMU.hpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_IMU_IMU_HPP_
#define DRIVERS_IMU_IMU_HPP_

#include "../../Context/Definitions.hpp"
#include "lsm6dsr_reg.h"

class IMU {
public:
  // m/s²
  static const float (&acceleration)[N_AXES_];

  // °/s
  static const float (&angularRate)[N_AXES_];

  // K
  static const float &temperature;

  static void initialize();
  static void update();

private:
  // IMU related
  static uint8_t      whoAmI;
  static uint8_t      reset;
  static stmdev_ctx_t context;

  // Raw data
  static int16_t rawAcceleration[N_AXES_];
  static int16_t rawAngularRate[N_ROTATION_AXES_];
  static int16_t rawTemperature;

  // Treated data
  static float acceleration_[N_AXES_];
  static float angularRate_[N_ROTATION_AXES_]; // NOLINT
  static float temperature_;                   // NOLINT

  static int32_t write(void *handle, uint8_t reg, const uint8_t *bufp,
                       uint16_t len);
  static int32_t read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
  static void    delay(uint32_t ms);
};

#endif /* DRIVERS_IMU_IMU_HPP_ */
