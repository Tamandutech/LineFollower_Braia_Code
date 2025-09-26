/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void cube_HAL_init(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SBat_Pin GPIO_PIN_0
#define SBat_GPIO_Port GPIOC
#define S15_Pin GPIO_PIN_1
#define S15_GPIO_Port GPIOC
#define S14_Pin GPIO_PIN_2
#define S14_GPIO_Port GPIOC
#define S13_Pin GPIO_PIN_3
#define S13_GPIO_Port GPIOC
#define S12_Pin GPIO_PIN_0
#define S12_GPIO_Port GPIOA
#define S11_Pin GPIO_PIN_1
#define S11_GPIO_Port GPIOA
#define S10_Pin GPIO_PIN_2
#define S10_GPIO_Port GPIOA
#define S9_Pin GPIO_PIN_3
#define S9_GPIO_Port GPIOA
#define S8_Pin GPIO_PIN_4
#define S8_GPIO_Port GPIOA
#define S7_Pin GPIO_PIN_5
#define S7_GPIO_Port GPIOA
#define S6_Pin GPIO_PIN_6
#define S6_GPIO_Port GPIOA
#define S5_Pin GPIO_PIN_7
#define S5_GPIO_Port GPIOA
#define S4_Pin GPIO_PIN_4
#define S4_GPIO_Port GPIOC
#define S3_Pin GPIO_PIN_5
#define S3_GPIO_Port GPIOC
#define S2_Pin GPIO_PIN_0
#define S2_GPIO_Port GPIOB
#define S1_Pin GPIO_PIN_1
#define S1_GPIO_Port GPIOB
#define S0_Pin GPIO_PIN_2
#define S0_GPIO_Port GPIOB
#define motor2pwm_Pin GPIO_PIN_6
#define motor2pwm_GPIO_Port GPIOC
#define motor2dir_Pin GPIO_PIN_7
#define motor2dir_GPIO_Port GPIOC
#define motor1pwm_Pin GPIO_PIN_8
#define motor1pwm_GPIO_Port GPIOC
#define motor1dir_Pin GPIO_PIN_9
#define motor1dir_GPIO_Port GPIOC
#define BTStatus_Pin GPIO_PIN_8
#define BTStatus_GPIO_Port GPIOA
#define EncEsqA_Pin GPIO_PIN_11
#define EncEsqA_GPIO_Port GPIOA
#define EncEsqB_Pin GPIO_PIN_12
#define EncEsqB_GPIO_Port GPIOA
#define pwm_coreless_Pin GPIO_PIN_12
#define pwm_coreless_GPIO_Port GPIOC
#define EncDirA_Pin GPIO_PIN_4
#define EncDirA_GPIO_Port GPIOB
#define EncDirB_Pin GPIO_PIN_5
#define EncDirB_GPIO_Port GPIOB
#define Boot_sw_Pin GPIO_PIN_8
#define Boot_sw_GPIO_Port GPIOB
#define IMU_INT_Pin GPIO_PIN_9
#define IMU_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
