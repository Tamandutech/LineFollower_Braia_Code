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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Leds_Pin GPIO_PIN_13
#define Leds_GPIO_Port GPIOC
#define S2_Pin GPIO_PIN_0
#define S2_GPIO_Port GPIOC
#define S3_Pin GPIO_PIN_1
#define S3_GPIO_Port GPIOC
#define S4_Pin GPIO_PIN_2
#define S4_GPIO_Port GPIOC
#define S6_Pin GPIO_PIN_0
#define S6_GPIO_Port GPIOA
#define S7_Pin GPIO_PIN_1
#define S7_GPIO_Port GPIOA
#define S8_Pin GPIO_PIN_2
#define S8_GPIO_Port GPIOA
#define S9_Pin GPIO_PIN_3
#define S9_GPIO_Port GPIOA
#define S11_Pin GPIO_PIN_5
#define S11_GPIO_Port GPIOA
#define S12_Pin GPIO_PIN_6
#define S12_GPIO_Port GPIOA
#define S13_Pin GPIO_PIN_7
#define S13_GPIO_Port GPIOA
#define S14_Pin GPIO_PIN_4
#define S14_GPIO_Port GPIOC
#define S15_Pin GPIO_PIN_5
#define S15_GPIO_Port GPIOC
#define PwmMotorEsqA_Pin GPIO_PIN_6
#define PwmMotorEsqA_GPIO_Port GPIOC
#define PwmMotorEsqB_Pin GPIO_PIN_7
#define PwmMotorEsqB_GPIO_Port GPIOC
#define PwmMotorDirA_Pin GPIO_PIN_8
#define PwmMotorDirA_GPIO_Port GPIOC
#define PwmMotorDirB_Pin GPIO_PIN_9
#define PwmMotorDirB_GPIO_Port GPIOC
#define BTStatus_Pin GPIO_PIN_8
#define BTStatus_GPIO_Port GPIOA
#define EncDirA_Pin GPIO_PIN_11
#define EncDirA_GPIO_Port GPIOA
#define EncDirB_Pin GPIO_PIN_12
#define EncDirB_GPIO_Port GPIOA
#define Buzzer_Pin GPIO_PIN_12
#define Buzzer_GPIO_Port GPIOC
#define NFautlDir_Pin GPIO_PIN_2
#define NFautlDir_GPIO_Port GPIOD
#define NFautlEsq_Pin GPIO_PIN_3
#define NFautlEsq_GPIO_Port GPIOB
#define EncEsqA_Pin GPIO_PIN_4
#define EncEsqA_GPIO_Port GPIOB
#define EncEsqB_Pin GPIO_PIN_5
#define EncEsqB_GPIO_Port GPIOB
#define IMU_INT_Pin GPIO_PIN_9
#define IMU_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
