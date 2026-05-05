/*
 * PeripheralsEnv.hpp
 *
 *  Created on: May 2, 2026
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_PERIPHERALSENV_HPP_
#define CONTEXT_PERIPHERALSENV_HPP_

#ifdef EXPOSE_IMU_PERIPHERAL
#include "i2c.h"
#endif

#ifdef EXPOSE_BLE_PERIPHERAL
#include "usart.h"
#endif

#if defined(EXPOSE_ENCODERS_PERIPHERAL) || defined(EXPOSE_LEDS_PERIPHERAL) || \
    defined(EXPOSE_MOTORS_PERIPHERAL) || defined(EXPOSE_VACUUM_PERIPHERAL) || \
    defined(EXPOSE_GLOBAL_PERIPHERALS)
#include "tim.h"
#endif

#ifdef EXPOSE_IR_SENSORS_PERIPHERALS
#include "adc.h"
#endif

#ifdef EXPOSE_GLOBAL_PERIPHERALS
#include "adc.h"
#endif

/*
 * Note: If using C++17 or later, prefer using "inline constexpr" for constants,
 * in order to avoid it being copied on every file this header is included.
 */
namespace PeripheralsEnv {
// Encoders
#ifdef EXPOSE_ENCODERS_PERIPHERAL
constexpr TIM_HandleTypeDef *ENCODER_LEFT_TIMER    = &htim4;
const uint32_t               ENCODER_LEFT_CHANNEL  = TIM_CHANNEL_ALL;
constexpr TIM_HandleTypeDef *ENCODER_RIGHT_TIMER   = &htim3;
const uint32_t               ENCODER_RIGHT_CHANNEL = TIM_CHANNEL_ALL;
#endif

// IMU
#ifdef EXPOSE_IMU_PERIPHERAL
constexpr I2C_HandleTypeDef *IMU_BUS = &hi2c1;
#endif

// Leds
#ifdef EXPOSE_LEDS_PERIPHERAL
TIM_HandleTypeDef *LEDS_TIMER   = &htim1;
const uint32_t     LEDS_CHANNEL = TIM_CHANNEL_1;
#endif

// Motors
#ifdef EXPOSE_MOTORS_PERIPHERAL
GPIO_TypeDef  *MOTOR_LEFT_DIRECTION_PORT        = motor2dir_GPIO_Port; // NOLINT
const uint16_t MOTOR_LEFT_DIRECTION_PIN         = motor2dir_Pin;
constexpr TIM_HandleTypeDef *MOTOR_LEFT_TIMER   = &htim8;
const uint32_t               MOTOR_LEFT_CHANNEL = TIM_CHANNEL_1;

GPIO_TypeDef  *MOTOR_RIGHT_DIRECTION_PORT      = motor1dir_GPIO_Port; // NOLINT
const uint16_t MOTOR_RIGHT_DIRECTION_PIN       = motor1dir_Pin;
constexpr TIM_HandleTypeDef *MOTOR_RIGHT_TIMER = &htim8;
const uint32_t               MOTOR_RIGHT_CHANNEL = TIM_CHANNEL_3;
#endif

// Vacuum
#ifdef EXPOSE_VACUUM_PERIPHERAL
constexpr TIM_HandleTypeDef *VACUUM_TIMER   = &htim5;
const uint32_t               VACUUM_CHANNEL = TIM_CHANNEL_2;
#endif

// BLE
#ifdef EXPOSE_BLE_PERIPHERAL
// TODO remove static, replace by "inline"
static UART_HandleTypeDef *BLE_BUS = &huart1;
#endif

#ifdef EXPOSE_GLOBAL_PERIPHERALS
constexpr TIM_HandleTypeDef *BASE_TIMER = &htim2;
constexpr ADC_HandleTypeDef *ADC_1      = &hadc1;
constexpr ADC_HandleTypeDef *ADC_2      = &hadc2;
#endif
} // namespace PeripheralsEnv

#endif /* CONTEXT_PERIPHERALSENV_HPP_ */
