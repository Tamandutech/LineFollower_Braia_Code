
#include "imu.h"

// priavte variables
static uint8_t whoamI, rst;

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route) {
    imu_ctx->write_reg = write_imu;
    imu_ctx->read_reg = read_imu;
    imu_ctx->mdelay = delay_ms;
    imu_ctx->handle = &IMU_BUS;

    while (1) {
        ble_log("Conectando com a IMU...\n", 25);
        lsm6dsr_device_id_get(imu_ctx, &whoamI);
        if (whoamI != LSM6DSR_ID) {
            ble_log("Error: Device ID mismatch\n", 27);
            delay_ms(500);
        } else {
            ble_log("IMU conectada\n", 15);
            break;
        }
    }

    lsm6dsr_reset_set(imu_ctx, PROPERTY_ENABLE);
    do {
        lsm6dsr_reset_get(imu_ctx, &rst);
    } while (rst);

    // INIT PARA POLLING
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