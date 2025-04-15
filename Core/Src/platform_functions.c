#include "platform_functions.h"

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

/** Please note that is MANDATORY: return 0 -> no Error.**/
int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
}
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg,
                            I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
}
