/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "lsm6dsr_platform_init.h"
#include "lsm6dsr_reg.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t a = 0;
uint8_t tx_buffer[1000] = "Hello World!\n";

stmdev_ctx_t imu_ctx;
lsm6dsr_pin_int1_route_t int1_route;
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM8_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM5_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    imu_init(&imu_ctx, &int1_route);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    // send Hello World! to UART
    platform_log(tx_buffer, 14);

    HAL_GPIO_WritePin(motor1a_GPIO_Port, motor1a_Pin, 0);
    HAL_GPIO_WritePin(motor1b_GPIO_Port, motor1b_Pin, 0);
    HAL_GPIO_WritePin(motor2a_GPIO_Port, motor2a_Pin, 0);
    HAL_GPIO_WritePin(motor2b_GPIO_Port, motor2b_Pin, 0);

    while (1) {
        // TESTE PONTE H
        // strcpy(tx_buffer, "Liga ponte H\n");
        // HAL_UART_Transmit(&huart1, tx_buffer, 14, 100);
        // HAL_GPIO_TogglePin(motor1b_GPIO_Port, motor1b_Pin);
        // HAL_GPIO_TogglePin(motor2b_GPIO_Port, motor2b_Pin);
        // HAL_Delay(1000);
        // strcpy(tx_buffer, "Desliga ponte H e inverte sentido\n");
        // HAL_UART_Transmit(&huart1, tx_buffer, 35, 100);
        // HAL_GPIO_TogglePin(motor1b_GPIO_Port, motor1b_Pin);
        // HAL_GPIO_TogglePin(motor2b_GPIO_Port, motor2b_Pin);
        // HAL_GPIO_TogglePin(motor1a_GPIO_Port, motor1a_Pin);
        // HAL_GPIO_TogglePin(motor2a_GPIO_Port, motor2a_Pin);
        // HAL_Delay(1000);
        // strcpy(tx_buffer, "Liga ponte H\n");
        // HAL_UART_Transmit(&huart1, tx_buffer, 14, 100);
        // HAL_GPIO_TogglePin(motor1b_GPIO_Port, motor1b_Pin);
        // HAL_GPIO_TogglePin(motor2b_GPIO_Port, motor2b_Pin);
        // HAL_Delay(1000);
        // strcpy(tx_buffer, "Desliga ponte H e inverte sentido\n");
        // HAL_UART_Transmit(&huart1, tx_buffer, 35, 100);
        // HAL_GPIO_TogglePin(motor1b_GPIO_Port, motor1b_Pin);
        // HAL_GPIO_TogglePin(motor2b_GPIO_Port, motor2b_Pin);
        // HAL_GPIO_TogglePin(motor1a_GPIO_Port, motor1a_Pin);
        // HAL_GPIO_TogglePin(motor2a_GPIO_Port, motor2a_Pin);
        // HAL_Delay(1000);

        // TESTE IMU 6D
        // lsm6dsr_all_sources_t all_source;
        // /* Check if 6D/4D Orientation events. */
        // lsm6dsr_all_sources_get(&imu_ctx, &all_source);

        // if (all_source.d6d_src.d6d_ia) {
        //     snprintf((char *)tx_buffer, sizeof(tx_buffer), "6D Or. switched to ");

        //     if (all_source.d6d_src.xh) {
        //         strcat((char *)tx_buffer, "XH");
        //     }

        //     if (all_source.d6d_src.xl) {
        //         strcat((char *)tx_buffer, "XL");
        //     }

        //     if (all_source.d6d_src.yh) {
        //         strcat((char *)tx_buffer, "YH");
        //     }

        //     if (all_source.d6d_src.yl) {
        //         strcat((char *)tx_buffer, "YL");
        //     }

        //     if (all_source.d6d_src.zh) {
        //         strcat((char *)tx_buffer, "ZH");
        //     }

        //     if (all_source.d6d_src.zl) {
        //         strcat((char *)tx_buffer, "ZL");
        //     }

        //     strcat((char *)tx_buffer, "\r\n");
        //     platform_log(tx_buffer, strlen((char const *)tx_buffer));
        // }

        // TESTE IMU POLLING
        uint8_t reg;
        /* Read output only if new xl value is available */
        lsm6dsr_xl_flag_data_ready_get(&imu_ctx, &reg);

        if (reg) {
            /* Read acceleration field data */
            memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
            lsm6dsr_acceleration_raw_get(&imu_ctx, data_raw_acceleration);
            acceleration_mg[0] =
                lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
            acceleration_mg[1] =
                lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
            acceleration_mg[2] =
                lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);
            snprintf((char *)tx_buffer, sizeof(tx_buffer),
                     "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                     acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
            platform_log(tx_buffer, strlen((char const *)tx_buffer));
        }

        lsm6dsr_gy_flag_data_ready_get(&imu_ctx, &reg);

        if (reg) {
            /* Read angular rate field data */
            memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
            lsm6dsr_angular_rate_raw_get(&imu_ctx, data_raw_angular_rate);
            angular_rate_mdps[0] =
                lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
            angular_rate_mdps[1] =
                lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
            angular_rate_mdps[2] =
                lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);
            snprintf((char *)tx_buffer, sizeof(tx_buffer),
                     "Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
                     angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
            platform_log(tx_buffer, strlen((char const *)tx_buffer));
        }

        lsm6dsr_temp_flag_data_ready_get(&imu_ctx, &reg);

        if (reg) {
            /* Read temperature data */
            memset(&data_raw_temperature, 0x00, sizeof(int16_t));
            lsm6dsr_temperature_raw_get(&imu_ctx, &data_raw_temperature);
            temperature_degC = lsm6dsr_from_lsb_to_celsius(
                data_raw_temperature);
            snprintf((char *)tx_buffer, sizeof(tx_buffer),
                     "Temperature [degC]:%6.2f\r\n", temperature_degC);
            platform_log(tx_buffer, strlen((char const *)tx_buffer));
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV8;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
