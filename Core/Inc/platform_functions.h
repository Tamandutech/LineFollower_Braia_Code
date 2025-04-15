#ifndef PLATFORM_FUNCTIONS_H
#define PLATFORM_FUNCTIONS_H

// includes necessários para o funcionamento do projeto
#include <stdint.h>

#include "lsm6dsr_reg.h"

// include especificos de hardware
#include "stm32g4xx_hal.h"

// Definições específicas de hardware
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

// Definições necessárias para o funcionamento do projeto
#define IMU_BUS hi2c1
#define BLE_BUS huart1

typedef struct pinhandler_t {
    GPIO_TypeDef *port;
    uint16_t pin;
} pinhandler_t;

void delay_ms(uint32_t millisec);

void ble_log(uint8_t *tx_buffer, uint16_t len);

uint8_t read_pin(pinhandler_t pin);
void write_pin(pinhandler_t pin, uint8_t state);
void toggle_pin(pinhandler_t pin);

int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

#endif /* PLATFORM_FUNCTIONS_H */
