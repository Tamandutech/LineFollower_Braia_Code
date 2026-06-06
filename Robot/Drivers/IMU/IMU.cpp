/*
 * IMUDriver.cpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

/******************************************************************************/
// INCLUDES
#include "IMU.hpp"
#include <cstring>

#define EXPOSE_IMU_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"

#include "../../Context/RobotEnv.hpp"

#include "../../Drivers/Leds/Leds.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

#include "stm32g4xx_hal.h"


/******************************************************************************/
// DEFINES
// The delay between two sensors readings while calibrating
#define SAMPLING_DELAY 1 // ms

// How many samples we are getting to determine min and max sensors values
#define SAMPLES 1500


/******************************************************************************/
// VARIABLES
static Logger *logger = new Logger("IMU", true, Logger::None);

// Private
uint8_t      IMU::whoAmI_  = 0;
stmdev_ctx_t IMU::context_ = {0};

int16_t IMU::rawAcceleration_[N_AXES_]              = {0};
int16_t IMU::rawAccelerationNoise_[N_AXES_]         = {0};
int16_t IMU::rawAngularRate_[N_ROTATION_AXES_]      = {0};
int16_t IMU::rawAngularRateNoise_[N_ROTATION_AXES_] = {0};
int16_t IMU::rawTemperature_                        = 0;

float IMU::previousAcceleration_[N_AXES_] = {0};
float IMU::acceleration_[N_AXES_]         = {0};
float IMU::previousSpeed_[N_AXES_]        = {0};
float IMU::speed_[N_AXES_]                = {0};
float IMU::position_[N_AXES_]             = {0};

float IMU::previousAngularRate_[N_ROTATION_AXES_] = {0};
float IMU::angularRate_[N_ROTATION_AXES_]         = {0};
float IMU::angle_[N_ROTATION_AXES_]               = {0};

float IMU::temperature_ = 0;

// Public
const float (&IMU::acceleration)[N_AXES_] = acceleration_;
const float (&IMU::speed)[N_AXES_]        = speed_;
const float (&IMU::position)[N_AXES_]     = position_;

const float (&IMU::angularRate)[N_ROTATION_AXES_] = angularRate_;
const float (&IMU::angle)[N_ROTATION_AXES_]       = angle_;

const float &IMU::temperature = temperature_;

int32_t IMU::writeRegister(void *handle, uint8_t reg, const uint8_t *bufp,
                           uint16_t len) {
  return HAL_I2C_Mem_Write(static_cast<I2C_HandleTypeDef *>(handle),
                           LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT,
                           (uint8_t *)bufp, // NOLINT
                           len, 1000);
}

int32_t IMU::readRegister(void *handle, uint8_t reg, uint8_t *bufp,
                          uint16_t len) {
  return HAL_I2C_Mem_Read(static_cast<I2C_HandleTypeDef *>(handle),
                          LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp,
                          len, 1000);
}

void IMU::delay(uint32_t ms) { HAL_Delay(ms); }

void IMU::initialize() {
  static bool initialized = false;
  uint8_t     reset       = 0;


  if(initialized) {
    logger->error("IMU already initialized, unexpected behaviour.");
    return;
  }

  context_.write_reg = writeRegister;
  context_.read_reg  = readRegister;
  context_.mdelay    = delay;
  context_.handle    = PeripheralsEnv::IMU_BUS;

  while(1) {
    logger->info("Connecting with IMU...");

    lsm6dsr_device_id_get(&context_, &whoAmI_);
    if(whoAmI_ != LSM6DSR_ID) {
      logger->error("Device ID mismatch");
      Timer::delayMiliseconds(500);
    } else {
      logger->info("IMU connected!");
      break;
    }
  }

  lsm6dsr_reset_set(&context_, PROPERTY_ENABLE);
  do {
    lsm6dsr_reset_get(&context_, &reset);
  } while(reset);

  // Init for polling
  // Disable I3C interface
  lsm6dsr_i3c_disable_set(&context_, LSM6DSR_I3C_DISABLE);

  // Enable Block Data Update
  lsm6dsr_block_data_update_set(&context_, PROPERTY_ENABLE);

  // Set Output Data Rate
  lsm6dsr_xl_data_rate_set(&context_, LSM6DSR_XL_ODR_12Hz5);
  lsm6dsr_gy_data_rate_set(&context_, LSM6DSR_GY_ODR_12Hz5);

  // Set full scale
  lsm6dsr_xl_full_scale_set(&context_, LSM6DSR_2g);
  lsm6dsr_gy_full_scale_set(&context_, LSM6DSR_2000dps);

  // Configure filtering chain (No aux interface)
  // Accelerometer - LPF1 + LPF2 path
  lsm6dsr_xl_hp_path_on_out_set(&context_, LSM6DSR_LP_ODR_DIV_100);
  lsm6dsr_xl_filter_lp2_set(&context_, PROPERTY_ENABLE);

  initialized = true;
}

void IMU::readRawValues() {
  uint8_t reg = 0;

  // (I) ACCELERATION
  // Read output only if new xl value is available
  lsm6dsr_xl_flag_data_ready_get(&context_, &reg);
  if(reg) {
    // Read acceleration field data
    memset(static_cast<void *>(rawAcceleration_), 0x00, 3 * sizeof(int16_t));
    lsm6dsr_acceleration_raw_get(&context_,
                                 static_cast<int16_t *>(rawAcceleration_));
  }

  // (II) ANGULAR RATE
  // Read output only if new gy value is available
  lsm6dsr_gy_flag_data_ready_get(&context_, &reg);
  if(reg) {
    // Read angular rate field data
    memset(static_cast<void *>(rawAngularRate_), 0x00, 3 * sizeof(int16_t));
    lsm6dsr_angular_rate_raw_get(&context_,
                                 static_cast<int16_t *>(rawAngularRate_));
  }
  
  // (III) TEMPERATURE
  // Read output only if new temperature value is available
  lsm6dsr_temp_flag_data_ready_get(&context_, &reg);
  if(reg) {
    // Read temperature data
    memset(&rawTemperature_, 0x00, sizeof(int16_t));
    lsm6dsr_temperature_raw_get(&context_, &rawTemperature_);
  }
}

void IMU::calibrate() {
  Leds::setColorFor(CenterLed, Orange);

  // (I) RESET VALUES
  for(uint8_t i = 0; i < N_AXES_; i++)
    rawAccelerationNoise_[i] = 0;

  for(uint8_t i = 0; i < N_ROTATION_AXES_; i++)
    rawAngularRateNoise_[i] = 0;

  // (II) MAKE A SUM OF THE READING DATA FROM SENSORS
  for(uint16_t s = 0; s < SAMPLES; s++) {
    readRawValues();

    for(uint8_t i = 0; i < N_AXES_; i++)
      rawAccelerationNoise_[i] += rawAcceleration_[i];

    for(uint8_t i = 0; i < N_ROTATION_AXES_; i++)
      rawAngularRateNoise_[i] += rawAngularRate_[i];

    delay(SAMPLING_DELAY);
  }

  // (III) FINALLY, CALCULATE THE AVERAGE
  for(uint8_t i = 0; i < N_AXES_; i++)
    rawAccelerationNoise_[i] /= SAMPLES;

  for(uint8_t i = 0; i < N_ROTATION_AXES_; i++)
    rawAngularRateNoise_[i] /= SAMPLES;
}

void IMU::update(const uint32_t dt) {
  readRawValues();

  // (I) ACCELERATION, SPEED AND POSITION
  for(uint8_t i = 0; i < N_AXES_; i++) {
    /*
     * The default value is in mg (milli-g), which is g/1000
     * Multipling by MILLI_GRAVITY, we get the value in m/s²
     */
    acceleration_[i] = lsm6dsr_from_fs2g_to_mg(rawAcceleration_[i] -
                                               rawAccelerationNoise_[i]) *
                       RobotEnv::MILLI_GRAVITY;

    // vᵢ = ∫a·dt
    speed_[i] +=
        0.5F * (previousAcceleration_[i] + acceleration_[i]) * (dt * 1e-6F);

    // xᵢ = ∬a·dt = ∫v·dt
    position_[i] += 0.5F * (previousSpeed_[i] + speed_[i]) * (dt * 1e-6F);

    previousAcceleration_[i] = acceleration_[i];
    previousSpeed_[i]        = speed_[i];
  }

  // (II) ANGULAR RATE AND ANGLE
  for(uint8_t i = 0; i < N_ROTATION_AXES_; i++) {
    /*
     * The default value is in mdps (milli degrees per second)
     * Dividing by 1000, we get the value in dps (°/s)
     */
    angularRate_[i] = lsm6dsr_from_fs2000dps_to_mdps(rawAngularRate_[i] -
                                                     rawAngularRateNoise_[i]) /
                      1000.0F;

    // θ = ∫ω·dt
    angle_[i] +=
        0.5F * (previousAngularRate_[i] + angularRate_[i]) * (dt * 1e-6F);

    previousAngularRate_[i] = angularRate_[i];
  }

  /*
   * (III) TEMPERATURE
   * The default value is in °C (Celsius)
   * Adding 273.15, we get the value in Kelvin
   *
   * [!] DO NOT REMOVE THE 273.15, KELVIN IS THE BEST UNIT [!]
   */
  temperature_ = lsm6dsr_from_lsb_to_celsius(rawTemperature_) + 273.15F;
}

void IMU::reset() {
  // Here we reset cumulative variables; on demand reading variables such as
  // acceleration and angular rate are not reseted
  for(uint8_t i = 0; i < N_AXES_; i++) {
    previousAcceleration_[i] = 0;

    previousSpeed_[i] = 0;
    speed_[i]         = 0;

    position_[i] = 0;
  }

  for(uint8_t i = 0; i < N_ROTATION_AXES_; i++) {
    previousAngularRate_[i] = 0;

    angle_[i] = 0;
  }
}
