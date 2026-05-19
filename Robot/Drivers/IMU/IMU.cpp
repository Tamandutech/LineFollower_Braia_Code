/*
 * IMUDriver.cpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#include "IMU.hpp"
#include <cstdint>
#include <cstring>
#include <sys/types.h>

#define EXPOSE_IMU_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"

#include "../../Context/RobotEnv.hpp"

#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

#include "stm32g4xx_hal.h"

static Logger *logger = new Logger("IMU", true, Logger::None);

// Private
uint8_t      IMU::whoAmI                           = 0;
uint8_t      IMU::reset                            = 0;
stmdev_ctx_t IMU::context                          = {0};
int16_t      IMU::rawAcceleration[N_AXES_]         = {0};
int16_t      IMU::rawAngularRate[N_ROTATION_AXES_] = {0};
int16_t      IMU::rawTemperature                   = 0;
float        IMU::acceleration_[N_AXES_]           = {0};
float        IMU::angularRate_[N_ROTATION_AXES_]   = {0};
float        IMU::temperature_                     = 0;

// Public
const float (&IMU::acceleration)[N_AXES_] = acceleration_;
const float (&IMU::angularRate)[N_AXES_]  = angularRate_;
const float &IMU::temperature             = temperature_;

int32_t IMU::write(void *handle, uint8_t reg, const uint8_t *bufp,
                   uint16_t len) {
  return HAL_I2C_Mem_Write(static_cast<I2C_HandleTypeDef *>(handle),
                           LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT,
                           (uint8_t *)bufp, // NOLINT
                           len, 1000);
}

int32_t IMU::read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  return HAL_I2C_Mem_Read(static_cast<I2C_HandleTypeDef *>(handle),
                          LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp,
                          len, 1000);
}

void IMU::delay(uint32_t ms) { HAL_Delay(ms); }

void IMU::initialize() {
  static bool initialized = false;

  if(initialized) {
    logger->error("IMU already initialized, unexpected behaviour.");
    return;
  }

  context.write_reg = write;
  context.read_reg  = read;
  context.mdelay    = delay;
  context.handle    = PeripheralsEnv::IMU_BUS;

  while(1) {
    logger->info("Connecting with IMU...");

    lsm6dsr_device_id_get(&context, &whoAmI);
    if(whoAmI != LSM6DSR_ID) {
      logger->error("Device ID mismatch");
      Timer::delayMiliseconds(500);
    } else {
      logger->info("IMU connected!");
      break;
    }
  }

  lsm6dsr_reset_set(&context, PROPERTY_ENABLE);
  do {
    lsm6dsr_reset_get(&context, &reset);
  } while(reset);

  // Init for polling
  // Disable I3C interface
  lsm6dsr_i3c_disable_set(&context, LSM6DSR_I3C_DISABLE);

  // Enable Block Data Update
  lsm6dsr_block_data_update_set(&context, PROPERTY_ENABLE);

  // Set Output Data Rate
  lsm6dsr_xl_data_rate_set(&context, LSM6DSR_XL_ODR_12Hz5);
  lsm6dsr_gy_data_rate_set(&context, LSM6DSR_GY_ODR_12Hz5);

  // Set full scale
  lsm6dsr_xl_full_scale_set(&context, LSM6DSR_2g);
  lsm6dsr_gy_full_scale_set(&context, LSM6DSR_2000dps);


  // Configure filtering chain (No aux interface)
  // Accelerometer - LPF1 + LPF2 path
  lsm6dsr_xl_hp_path_on_out_set(&context, LSM6DSR_LP_ODR_DIV_100);
  lsm6dsr_xl_filter_lp2_set(&context, PROPERTY_ENABLE);

  initialized = true;
}

void IMU::update() {
  uint8_t reg = 0;

  // Read output only if new xl value is available
  lsm6dsr_xl_flag_data_ready_get(&context, &reg);
  if(reg) {
    // Read acceleration field data
    memset(static_cast<void *>(rawAcceleration), 0x00, 3 * sizeof(int16_t));
    lsm6dsr_acceleration_raw_get(&context,
                                 static_cast<int16_t *>(rawAcceleration));

    /*
     * The default value is in mg (milli-g), which is g/1000
     * Multipling by MILLI_GRAVITY, we get the value in m/s²
     */
    acceleration_[X] =
        lsm6dsr_from_fs2g_to_mg(rawAcceleration[X]) * RobotEnv::MILLI_GRAVITY;
    acceleration_[Y] =
        lsm6dsr_from_fs2g_to_mg(rawAcceleration[Y]) * RobotEnv::MILLI_GRAVITY;
    acceleration_[Z] =
        lsm6dsr_from_fs2g_to_mg(rawAcceleration[Z]) * RobotEnv::MILLI_GRAVITY;

    logger->debug("Acceleration [g]: %4.2f, %4.2f, %4.2f", acceleration_[X],
                  acceleration_[Y], acceleration_[Z]);
  }

  // Read output only if new gy value is available
  lsm6dsr_gy_flag_data_ready_get(&context, &reg);
  if(reg) {
    // Read angular rate field data
    memset(static_cast<void *>(rawAngularRate), 0x00, 3 * sizeof(int16_t));
    lsm6dsr_angular_rate_raw_get(&context,
                                 static_cast<int16_t *>(rawAngularRate));

    /*
     * The default value is in mdps (milli degrees per second)
     * Dividing by 1000, we get the value in dps (°/s)
     */
    angularRate_[Pitch] =
        lsm6dsr_from_fs2000dps_to_mdps(rawAngularRate[Pitch]) / 1000.0F;
    angularRate_[Row] =
        lsm6dsr_from_fs2000dps_to_mdps(rawAngularRate[Row]) / 1000.0F;
    angularRate_[Yaw] =
        lsm6dsr_from_fs2000dps_to_mdps(rawAngularRate[Yaw]) / 1000.0F;

    logger->debug("Angular rate [mdps]: %4.2f, %4.2f, %4.2f",
                  angularRate_[Pitch], angularRate_[Row], angularRate_[Yaw]);
  }

  // Read output only if new temperature value is available
  lsm6dsr_temp_flag_data_ready_get(&context, &reg);
  if(reg) {
    // Read temperature data
    memset(&rawTemperature, 0x00, sizeof(int16_t));
    lsm6dsr_temperature_raw_get(&context, &rawTemperature);

    /*
     * The default value is in C (Celcius)
     * Adding 273.15, we get the value in Kelvin
     *
     * [!] DO NOT REMOVE THE 273.15, KELVIN IS THE BEST UNIT [!]
     */
    temperature_ = lsm6dsr_from_lsb_to_celsius(rawTemperature) + 273.15F;

    logger->debug("Temperature [K]: %6.2f", temperature_);
  }
}
