/*
 * main.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "application.h"

#include "adc.h"
#include "tim.h"

#include "Services/BLEListener/ble_listener.h"

#include "Utils/Battery.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Timer.hpp"

#include "Drivers/EncoderDriver/EncoderDriver.hpp"
#include "Drivers/LedDriver/LedDriver.hpp"
#include "Drivers/MotorDriver/MotorDriver.hpp"
#include "Drivers/QTRSensorDriver/QTRSensorDriver.hpp"
#include "Drivers/VacuumDriver/VacuumDriver.hpp"

/*
 * Here we declare private (aka static) variables to this file, but they are
 * still sharede between functions
 */
static Logger *logger = new Logger("Main", true, Logger::Level::All);

void setup(void) {
  logger->info("Robot is starting...");

  // Init timer
  HAL_TIM_Base_Start(&htim2);

  // Init PWM
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);

  // Init encoders
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  // Enables overflow/underflow interrupt
  __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);

  // Init ADC
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_Delay(100);

  // Init DMA
  HAL_ADC_Start_DMA(&hadc1, adc1_buffer, 9);
  HAL_ADC_Start_DMA(&hadc2, adc2_buffer, 9);
  HAL_Delay(50);

  // Start DMA reception
  start_ble_listening();

  // Print battery information
  Timer::delayMiliseconds(25);
  logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  if(Battery::getBatteryVoltage() < 7.5F) {
    logger->warning("Low battery");
  }

  // TODO
  // imu_init(&imu_ctx, &int1_route);

  Timer::delayMiliseconds(25);
  logger->info("Robot started!");

  Timer::delayMiliseconds(1000);
  QTRSensorDriver::calibrateSensors();

  LedDriver::setColorForAll(LedDriver::Colors.white);
  EncoderDriver::reset();

  Timer::delayMiliseconds(25);
  logger->info("Waiting for run command...");
}

void loop(void) {
  if(run) {
    Timer::delayMiliseconds(1000);

    logger->debug("Encoders L:%05d R:%05d",
                  EncoderDriver::getCounter(EncoderDriver::Encoder::Left),
                  EncoderDriver::getCounter(EncoderDriver::Encoder::Right));
  }

  Timer::delayMiliseconds(1000);
}
