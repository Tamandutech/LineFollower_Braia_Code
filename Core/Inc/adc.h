/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

extern ADC_HandleTypeDef hadc2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_ADC1_Init(void);
void MX_ADC2_Init(void);

/* USER CODE BEGIN Prototypes */
// BUFFERS
#define ADC_BUFFER_SIZE 9
// Buffer for the ADC values
extern uint32_t adc1_buffer[ADC_BUFFER_SIZE];
extern uint32_t adc2_buffer[ADC_BUFFER_SIZE];

// SENSORS
enum _Sensor {
  // Left
  L_1 = 0,
  L_2,

  // Center
  C_1,
  C_2,
  C_3,
  C_4,
  C_5,
  C_6,
  C_7,
  C_8,
  C_9,
  C_10,
  C_11,
  C_12,

  // Right
  R_1,
  R_2,

  _N_SENSORS
};

/*
 * Since the ADC buffers are in a messy order, we create an array of pointers
 * to get the correct values in the correct order.
 * We just need to translate manually the addresses on the ".c" file.
 *
 * For example, to access the right encoder: *sensorValues[R_1]
 */
extern uint32_t *const rawSensorValues[_N_SENSORS];

// BATTERY
/*
 * The same happens here: we are going to expose a pointer to the desired
 * variable, matching the correct ADC address.
 */
extern uint32_t *const batteryVoltage;
extern uint32_t *const referenceVoltage;
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

