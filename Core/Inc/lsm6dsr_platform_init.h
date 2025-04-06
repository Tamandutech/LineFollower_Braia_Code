#ifndef LSM6DSR_PLATFORM_INIT_H
#define LSM6DSR_PLATFORM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "lsm6dsr_reg.h"
#include "stm32g4xx_hal.h"

/* DEFINIÇÕES DEPENDENTES DO HARDWARE
   Definições de hardware para o STM32G4xx com I2C e UART */
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
#define SENSOR_BUS hi2c1
#define LOG_BUS huart1

/* Funções de interface de comunicação com o sensor */
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
void platform_log(uint8_t *tx_buffer, uint16_t len);
void platform_delay(uint32_t millisec);

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route);

#ifdef __cplusplus
}
#endif

#endif /* LSM6DSR_PLATFORM_INIT_H */
