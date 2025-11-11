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
static Logger *logger = new Logger(
    "Main", true,
    static_cast<Logger::Level>(Logger::Level::Debug | Logger::Level::Info));

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

  // Print battery information
  Timer::delayMiliseconds(100);
  logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  if(Battery::getBatteryVoltage() < 7.5F) {
    logger->warning("Low battery");
  }

  // TODO
  // EncoderDriver::reset();

  // Start DMA reception
  start_ble_listening();

  Timer::delayMiliseconds(100);
  logger->info("Robot started!");
  // Start DMA reception

  // TODO tmp
  // imu_init(&imu_ctx, &int1_route);

  logger->info("Robot started!");

  QTRSensorDriver::calibrateSensors();

  LedDriver::setColorForAll(LedDriver::Colors.blue);
  Timer::delayMiliseconds(500);

  LedDriver::setColorFor(LedDriver::Colors.magenta, LedDriver::Leds::Center);
  Timer::delayMiliseconds(500);
}

void loop(void) {
  if(run) {
    logger->debug("Run == 1");
  }

  Timer::delayMiliseconds(1000);

  logger->debug("Encoders L:%05lu R:%05lu",
                EncoderDriver::getCounter(EncoderDriver::Encoder::Left),
                EncoderDriver::getCounter(EncoderDriver::Encoder::Right));
}
