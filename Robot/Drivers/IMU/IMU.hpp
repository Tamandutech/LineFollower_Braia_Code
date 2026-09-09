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
  // m/s
  static const float (&speed)[N_AXES_];
  // m
  static const float (&position)[N_AXES_];

  // °/s
  static const float (&angularRate)[N_ROTATION_AXES_];
  // °
  static const float (&angle)[N_ROTATION_AXES_];

  // K
  static const float &temperature;

  static void initialize();
  static void calibrate();
  static void reset();
  static void update(const uint32_t us_dt);

private:
  // IMU related
  static uint8_t      whoAmI_;
  static stmdev_ctx_t context_;

  // Raw data
  static int16_t rawAcceleration_[N_AXES_];
  static int16_t rawAccelerationNoise_[N_AXES_];
  static int16_t rawAngularRate_[N_ROTATION_AXES_];
  static int16_t rawAngularRateNoise_[N_ROTATION_AXES_];
  static int16_t rawTemperature_;

  // Treated data
  static float previousAcceleration_[N_AXES_];
  static float acceleration_[N_AXES_];
  static float previousSpeed_[N_AXES_];
  static float speed_[N_AXES_];
  static float position_[N_AXES_];

  static float previousAngularRate_[N_ROTATION_AXES_];
  static float angularRate_[N_ROTATION_AXES_];
  static float angle_[N_ROTATION_AXES_];

  static float temperature_;

  static int32_t writeRegister(void *handle, uint8_t reg, const uint8_t *bufp,
                               uint16_t len);
  static int32_t readRegister(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
  static void    delay(uint32_t ms);
  static void    readRawValues();
};

#endif /* DRIVERS_IMU_IMU_HPP_ */

// Telemann - Viola Concerto in G major, TWV 51:G9
