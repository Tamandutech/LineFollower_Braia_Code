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
#include "Services/Mapper/Mapper.hpp"

#include "Utils/Battery/Battery.hpp"
#include "Utils/Logger/Logger.hpp"
#include "Utils/Timer/Timer.hpp"

#include "Drivers/EncoderDriver/EncoderDriver.hpp"
#include "Drivers/LedDriver/LedDriver.hpp"
#include "Drivers/MotorDriver/MotorDriver.hpp"
#include "Drivers/QTRSensorDriver/QTRSensorDriver.hpp"
#include "Drivers/VacuumDriver/VacuumDriver.hpp"
#include <cstdint>

/*
 * Here we declare private (aka static) variables to this file, but they are
 * still sharede between functions
 */
static Logger  *logger   = new Logger("Main", true, Logger::Level::All);
static uint32_t lastTime = 0;

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

  // Enables overflow/underflow interrupt for encoders
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
  if(Battery::getBatteryVoltage() < 7.0F) {
    logger->warning("Low battery: %.2f V", Battery::getBatteryVoltage());
  } else {
    logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  }

  // TODO
  // imu_init(&imu_ctx, &int1_route);

  Timer::delayMiliseconds(25);
  logger->info("Robot started!");

  // Logger
  Logger::setShowTimestamp(false);

  Timer::delayMiliseconds(1000);
  QTRSensorDriver::calibrateSensors();

  EncoderDriver::reset();

  Timer::delayMiliseconds(25);
  // logger->info("Waiting for run command...");

  Mapper::map();
}

void loop(void) {
  // QTRSensorDriver::readCalibrated();
  // Timer::delayMiliseconds(500);  
  // logger->debug("[%03d %03d] %03d [%03d %03d]",
  //               QTRSensorDriver::sensorValues[QTRSensorDriver::L_1],
  //               QTRSensorDriver::sensorValues[QTRSensorDriver::L_2],
  //               QTRSensorDriver::sensorValues[QTRSensorDriver::C_6],
  //               QTRSensorDriver::sensorValues[QTRSensorDriver::R_1],
  //               QTRSensorDriver::sensorValues[QTRSensorDriver::R_2]);
  // Timer::delayMiliseconds(500);

  // if(run) {
  //   if(Timer::getMicroseconds() - lastTime >= 1000) {
  //     VacuumDriver::pwmOutput(350);
  //     MotorDriver::speedOutput(150);
  //     LedDriver::setColorForAll(LedDriver::Colors.red);

  //     lastTime = Timer::getMicroseconds();
  //   }
  // } else {
  //   MotorDriver::stop();
  //   VacuumDriver::pwmOutput(0);
  //   LedDriver::setColorForAll(LedDriver::Colors.green);
  // }
}
