#include "platform_functions.h"

// Functions to configure platform peripherals

volatile uint32_t adc1_buffer[8];
volatile uint32_t adc2_buffer[9];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        adc_buffer[13] = adc1_buffer[0];
        adc_buffer[12] = adc1_buffer[1];
        adc_buffer[11] = adc1_buffer[2];
        adc_buffer[10] = adc1_buffer[3];
        adc_buffer[16] = adc1_buffer[4];
        adc_buffer[15] = adc1_buffer[5];
        adc_buffer[2] = adc1_buffer[6];
        adc_buffer[3] = adc1_buffer[7];
        adc_buffer[0] = adc1_buffer[8];  // Referência interterna
    } else if (hadc->Instance == ADC2) {
        adc_buffer[7] = adc2_buffer[0];
        adc_buffer[6] = adc2_buffer[1];
        adc_buffer[5] = adc2_buffer[2];
        adc_buffer[14] = adc2_buffer[3];
        adc_buffer[4] = adc2_buffer[4];
        adc_buffer[1] = adc2_buffer[5];
        adc_buffer[8] = adc2_buffer[6];
        adc_buffer[9] = adc2_buffer[7];
        adc_buffer[17] = adc2_buffer[8];  // Bateria
    }
}

// Functions to be used in the main loop
void delay_ms(uint32_t millisec) {
    if (millisec == 0) {
        return;
    }
    HAL_Delay(millisec - 1);
}

void ble_log(uint8_t *tx_buffer, uint16_t len) {
    HAL_UART_Transmit(&BLE_BUS, tx_buffer, len, 1000);  // trocar por dma
}

void write_pin(pinhandler_t pin, uint8_t state) {
    if (state) {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
    }
}

void toggle_pin(pinhandler_t pin) {
    HAL_GPIO_TogglePin(pin.port, pin.pin);
}

void adc_start(void) {
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_Delay(300);
    HAL_ADC_Start_DMA(&hadc1, adc1_buffer, 8);
    HAL_ADC_Start_DMA(&hadc2, adc2_buffer, 9);
    ble_log((uint8_t *)"ADC started\n", 13);
}

/** Please note that is MANDATORY: return 0 -> no Error.**/
int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
}
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg,
                            I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
}

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route) {
    uint8_t whoamI, rst;

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
