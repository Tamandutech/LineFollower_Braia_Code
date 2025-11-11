/*
 * imu.c
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "imu.h"

#include "lsm6dsr_reg.h"
#include "stm32g4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
#define IMU_BUS hi2c1

// TODO
static void delay_ms(uint32_t millisec) {
  uint32_t tickstart = HAL_GetTick();
  while((HAL_GetTick() - tickstart) < millisec) {
    // No Operation
    __NOP();
  }
}

/** Please note that is MANDATORY: return 0 -> no Error.**/
static int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp,
                         uint16_t len) {
  return HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT,
                           (uint8_t *)bufp, len, 1000);
}

static int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp,
                        uint16_t len) {
  return HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT,
                          bufp, len, 1000);
}

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route) {
  uint8_t whoamI, rst;

  imu_ctx->write_reg = write_imu;
  imu_ctx->read_reg  = read_imu;
  imu_ctx->mdelay    = delay_ms;
  imu_ctx->handle    = &IMU_BUS;

  while(1) {
    // TODO
    // bleLog("Connecting with IMU...\n");
    lsm6dsr_device_id_get(imu_ctx, &whoamI);
    if(whoamI != LSM6DSR_ID) {
      // TODO
      // bleLog("Error: Device ID mismatch\n");
      delay_ms(500);
    } else {
      // TODO
      // bleLog("IMU connected\n");
      break;
    }
  }

  lsm6dsr_reset_set(imu_ctx, PROPERTY_ENABLE);
  do {
    lsm6dsr_reset_get(imu_ctx, &rst);
  } while(rst);

  // Init for polling
  // Disable I3C interface
  lsm6dsr_i3c_disable_set(imu_ctx, LSM6DSR_I3C_DISABLE);

  // Enable Block Data Update
  lsm6dsr_block_data_update_set(imu_ctx, PROPERTY_ENABLE);

  // Set Output Data Rate
  lsm6dsr_xl_data_rate_set(imu_ctx, LSM6DSR_XL_ODR_12Hz5);
  lsm6dsr_gy_data_rate_set(imu_ctx, LSM6DSR_GY_ODR_12Hz5);

  // Set full scale
  lsm6dsr_xl_full_scale_set(imu_ctx, LSM6DSR_2g);
  lsm6dsr_gy_full_scale_set(imu_ctx, LSM6DSR_2000dps);

  /*
   * Configure filtering chain(No aux interface)
   * Accelerometer - LPF1 + LPF2 path
   */
  lsm6dsr_xl_hp_path_on_out_set(imu_ctx, LSM6DSR_LP_ODR_DIV_100);
  lsm6dsr_xl_filter_lp2_set(imu_ctx, PROPERTY_ENABLE);
}