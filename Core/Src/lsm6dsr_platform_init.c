
#include "lsm6dsr_platform_init.h"

// priavte variables
static uint8_t whoamI, rst;

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route) {
    imu_ctx->write_reg = platform_write;
    imu_ctx->read_reg = platform_read;
    imu_ctx->mdelay = platform_delay;
    imu_ctx->handle = &SENSOR_BUS;

     while(1) {
        platform_log("Conectando com a IMU...\n", 25);
        lsm6dsr_device_id_get(imu_ctx, &whoamI);
        if (whoamI != LSM6DSR_ID) {
            platform_log("Error: Device ID mismatch\n", 27);
            platform_delay(500);
        } else {
            platform_log("IMU conectada\n", 15);
            break;
        }
    }

    lsm6dsr_reset_set(imu_ctx, PROPERTY_ENABLE);
    do {
        lsm6dsr_reset_get(imu_ctx, &rst);
    } while (rst);

    // INIT PARA TESTE 6D
    // /* Disable I3C interface. */
    // lsm6dsr_i3c_disable_set(imu_ctx, LSM6DSR_I3C_DISABLE);
    // /* Set XL Output Data Rate to 417 Hz. */
    // lsm6dsr_xl_data_rate_set(imu_ctx, LSM6DSR_XL_ODR_416Hz);
    // /* Set 2g full XL scale. */
    // lsm6dsr_xl_full_scale_set(imu_ctx, LSM6DSR_2g);
    // /* Set threshold to 60 degrees. */
    // lsm6dsr_6d_threshold_set(imu_ctx, LSM6DSR_DEG_60);
    // /* LPF2 on 6D/4D function selection. */
    // lsm6dsr_xl_lp2_on_6d_set(imu_ctx, PROPERTY_ENABLE);
    // /* To enable 4D mode uncomment next line.
    //  * 4D orientation detection disable Z-axis events.
    //  */
    // lsm6dsr_4d_mode_set(imu_ctx, PROPERTY_ENABLE);
    // /* Uncomment if interrupt generation on Free Fall INT1 pin */
    // lsm6dsr_pin_int1_route_get(imu_ctx, int1_route);
    // int1_route->md1_cfg.int1_ff = PROPERTY_ENABLE;
    // lsm6dsr_pin_int1_route_set(imu_ctx, int1_route);

    // INIT PARA TESTE POLLING
    /* Disable I3C interface */
    lsm6dsr_i3c_disable_set(imu_ctx, LSM6DSR_I3C_DISABLE);
    /* Enable Block Data Update */
    lsm6dsr_block_data_update_set(imu_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate */
    lsm6dsr_xl_data_rate_set(imu_ctx, LSM6DSR_XL_ODR_12Hz5);
    lsm6dsr_gy_data_rate_set(imu_ctx, LSM6DSR_GY_ODR_12Hz5);
    /* Set full scale */
    lsm6dsr_xl_full_scale_set(imu_ctx, LSM6DSR_2g);
    lsm6dsr_gy_full_scale_set(imu_ctx, LSM6DSR_2000dps);
    /* Configure filtering chain(No aux interface)
     * Accelerometer - LPF1 + LPF2 path
     */
    lsm6dsr_xl_hp_path_on_out_set(imu_ctx, LSM6DSR_LP_ODR_DIV_100);
    lsm6dsr_xl_filter_lp2_set(imu_ctx, PROPERTY_ENABLE);
}

/** Please note that is MANDATORY: return 0 -> no Error.**/
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
}
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg,
                            I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
}

void platform_log(uint8_t *tx_buffer, uint16_t len) {
    HAL_UART_Transmit(&huart1, tx_buffer, len, 1000);
}

/** Optional (may be required by driver) **/
void platform_delay(uint32_t millisec) {
    HAL_Delay(millisec);
}
