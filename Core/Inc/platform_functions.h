#ifndef PLATFORM_FUNCTIONS_H
#define PLATFORM_FUNCTIONS_H

// Definições especificas de hardware
#include "stm32g4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

// Definições necessários para o funcionamento do projeto presente em arquivos interplataforma
#include <stdint.h>

#include "lsm6dsr_reg.h"

#define MILISEONDS HAL_GetTick()
#define MICROSECONDS (volatile uint32_t)((TIM2->CNT) / (uint32_t)10)
#define NANOSECONDS (volatile uint32_t)((TIM2->CNT) * 100U)

#define IMU_BUS hi2c1
#define BLE_BUS huart1

typedef struct pinhandler_t {
    GPIO_TypeDef *port;
    uint16_t pin;
} pinhandler_t;

typedef struct pwmhandler_t {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} pwmhandler_t;

void mcu_start(void);

void delay_ms(uint32_t millisec);
void delay_us(uint32_t microsec);
void delay_ns(uint32_t nanosec);

void ble_log(uint8_t *tx_buffer, uint16_t len);

uint8_t read_pin(pinhandler_t pin);
void write_pin(pinhandler_t pin, uint8_t state);
void toggle_pin(pinhandler_t pin);

void set_pwm(pwmhandler_t pwmpin, uint16_t dutty);

void update_encoder_value(int32_t *encoderArray);

void update_adc(uint32_t *adc_buffer);

int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route);

#endif /* PLATFORM_FUNCTIONS_H */
