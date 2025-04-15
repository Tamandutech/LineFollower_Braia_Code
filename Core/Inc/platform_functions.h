#ifndef PLATFORM_FUNCTIONS_H
#define PLATFORM_FUNCTIONS_H

// Definições especificas de hardware
#include "stm32g4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern volatile uint32_t adc_buffer[18];

// Definições necessários para o funcionamento do projeto
#include <stdint.h>

#include "lsm6dsr_reg.h"

#define IMU_BUS hi2c1
#define BLE_BUS huart1

typedef struct pinhandler_t {
    GPIO_TypeDef *port;
    uint16_t pin;
} pinhandler_t;

void delay_ms(uint32_t millisec);

void ble_log(uint8_t *tx_buffer, uint16_t len);

void write_pin(pinhandler_t pin, uint8_t state);
void toggle_pin(pinhandler_t pin);

void adc_start(void);

int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route);

#endif /* PLATFORM_FUNCTIONS_H */
