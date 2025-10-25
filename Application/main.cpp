/*
 * main.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "main.h"

// Peripheral headers for initialization
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

extern "C"
{
  // This function is defined/declared on main.c
  extern void SystemClock_Config(void);
}

int main()
{
  // Reset of all peripherals, Initializes the Flash interface and the Systick
  HAL_Init();

  // Configure the system clock
  SystemClock_Config();

  // Initialize all configured peripherals
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
  MX_TIM2_Init();
  MX_TIM1_Init();

  while (true)
  {
    // ...
  }
}



