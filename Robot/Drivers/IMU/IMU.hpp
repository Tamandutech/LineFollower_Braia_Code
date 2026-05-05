/*
 * IMU.hpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_IMU_IMU_HPP_
#define DRIVERS_IMU_IMU_HPP_

#include <cstdint>

#include "lsm6dsr_reg.h"

class IMU {
public:
  enum AccelerationAxes : uint8_t {
    X = 0,
    Y,
    Z,

    N_ACCELERATION_AXES_
  };

  enum AngularRateAxes : uint8_t {
    Omega_P = 0,
    Omega_R,
    Omega_Y,

    N_ANGULAR_RATE_AXES_
  };

  // m/s²
  static const volatile float (&acceleration)[N_ACCELERATION_AXES_];

  // °/s
  static const volatile float (&angularRate)[N_ACCELERATION_AXES_];

  // K
  static const volatile float &temperature;

  static void initialize();
  static void update();

private:
  // IMU related
  static uint8_t      whoAmI;
  static uint8_t      reset;
  static stmdev_ctx_t context;

  // Raw data
  static int16_t rawAcceleration[N_ACCELERATION_AXES_];
  static int16_t rawAngularRate[N_ANGULAR_RATE_AXES_];
  static int16_t rawTemperature;

  // Treated data
  static float acceleration_mg_[N_ACCELERATION_AXES_];
  static float angularRate_mdps_[N_ANGULAR_RATE_AXES_]; // NOLINT
  static float temperature_;                            // NOLINT

  static int32_t write(void *handle, uint8_t reg, const uint8_t *bufp,
                       uint16_t len);
  static int32_t read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
  static void    delay(uint32_t ms);
};

#endif /* DRIVERS_IMU_IMU_HPP_ */
