/*
 * main.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

// Base header
#include "robot.h"

// Standard headers
#include <cstdint>

// STM32 HAL headers
#include "adc.h"
#include "tim.h"

// Headers from our code base
#include "Context/GlobalData.hpp"
#include "Context/RobotEnv.hpp"

#include "Services/Mapper/Mapper.hpp"

#include "Utils/Battery/Battery.hpp"
#include "Utils/Logger/Logger.hpp"
#include "Utils/Timer/Timer.hpp"

#include "Drivers/Encoders/Encoders.hpp"
#include "Drivers/Leds/Leds.hpp"
#include "Drivers/Motors/Motors.hpp"
#include "Drivers/Vacuum/Vacuum.hpp"
#include "Services/BLE/BLE.hpp"

/*
 * Here we declare private (aka static) variables to this file, but they are
 * still shared between functions
 */
static Logger *logger = new Logger("Main", false, Logger::Level::All);

/*
 * Here we have private (aka static) functions to this file
 */
static void stopRunning() {
  Motors::stop();
  Vacuum::stopAfter(700);
}

static void startRunning() {
  uint8_t  i        = 1;
  uint32_t lastTime = 0;

  // Assert
  if(globalData.mapData.size() < 3) {
    logger->error("Map is too small: %d points", globalData.mapData.size());
    return;
  }

  // Prepare
  Leds::setColorForAll(Leds::Magenta);
  Vacuum::pwmAcceleratedOutput(RobotEnv::VACUUM_BASE_PWM);
  Encoders::reset();
  lastTime = Timer::getMicroseconds();
  logger->info("Running...");

  // Start
  while(globalData.action == Action::Run && i < globalData.mapData.size()) {
    if(Timer::getMicroseconds() - lastTime >= RobotEnv::BASE_LOOP_TIME_US) {
      int32_t avg = Encoders::getAverage();

      if(avg >= globalData.mapData[i - 1].encoderAverage &&
         avg < globalData.mapData[i].encoderAverage) {
        // At index i-1
        Motors::pwmOutput(globalData.mapData[i - 1].baseMotorPWM);
        Vacuum::pwmOutput(globalData.mapData[i - 1].baseVacuumPWM);
        Leds::setColorForAll(globalData.mapData[i - 1].colorIndex);
      } else if(avg >= globalData.mapData[i].encoderAverage) {
        // At index i
        Motors::pwmOutput(globalData.mapData[i].baseMotorPWM);
        Vacuum::pwmOutput(globalData.mapData[i].baseVacuumPWM);
        Leds::setColorForAll(globalData.mapData[i].colorIndex);

        logger->info("[%02d] Encoder: %04ld, motor: %03.0f, vacuum: %03.0f", i,
                     avg, // NOLINT
                     globalData.mapData[i].baseMotorPWM,
                     globalData.mapData[i].baseVacuumPWM);

        i++;
      }

      lastTime = Timer::getMicroseconds();
    }
  }

  // Stop graceffuly
  // This action will be performed only if the stop command wasn't sent
  if(globalData.action == Action::Run) {
    Motors::stop();
    logger->info("Stopping at encoder %04ld",
                 Encoders::getAverage()); // NOLINT
    Vacuum::stopAfter(2000);
  }
}

static void customAction() {
  /*
   * Implement here a custom action
   */
}

void setup(void) {
  // Logger
  Logger::setShowTimestamp(false);
  Logger::setShowLogLevel(true);
  Logger::setUseDoubleBreak(false);

  Logger::log("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
              "\nLast build: %s @ %s\n"
              "Robot is starting...\n",
              __DATE__, __TIME__);

  // Initialize timer
  HAL_TIM_Base_Start(&htim2);

  // Initialize PWM
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);

  // Initialize encoders
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  // Enables overflow/underflow interrupt for encoders
  __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);

  // Initialize ADC
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  HAL_Delay(100);

  // Initialize DMA
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_buffer, 9);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc2_buffer, 9);
  HAL_Delay(50);

  // Start DMA reception
  BLE::start();

  // Print battery information
  Timer::delayMiliseconds(75);
  if(Battery::getBatteryVoltage() < 7.8F) {
    logger->warning("Low battery: %.2f V", Battery::getBatteryVoltage());
  } else {
    logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  }

  // TODO
  // imu_init(&imu_ctx, &int1_route);

  Timer::delayMiliseconds(75);
  logger->info("Robot started!");

  // Reset LEDs
  Leds::setColorForAll(Leds::Black);

  // Calibrate sensors
  Timer::delayMiliseconds(1000);
  // QTRSensorDriver::calibrateSensors();

  // Reset encoders
  Encoders::reset();

  Timer::delayMiliseconds(25);
  logger->info("Waiting for run command...");

  ///////////////////////////// Manual mapping  ////////////////////////////////
  // Reset variables
  globalData.mapData.clear();
  globalData.markCount = 0;

  // Assign data
  globalData.mapData = {
      {0,       RobotEnv::MOTOR_BASE_PWM, RobotEnv::VACUUM_BASE_PWM, Leds::White  },

      {300000,  RobotEnv::MOTOR_BASE_PWM, RobotEnv::VACUUM_BASE_PWM,
       Leds::Green                                                                },

      // Last point should be the end of the track
      {1600000, 0,                        270,                       Leds::Magenta}
  };
  /****************************************************************************/
}

void loop(void) {
  static Timer inactivityTimer(Timer::Miliseconds);

  switch(globalData.action) {
  case Action::Run: startRunning(); break;

  case Action::Map: Mapper::map(); break;

  case Action::CustomAction: customAction(); break;

  case Action::None:
  default: stopRunning();
  }

  // Being pedantic to avoid that the robot runs when it shouldn't
  stopRunning();

  // Reset the action variable after the command was performed
  globalData.action = Action::None;

  // Blink LEDs to show inactivity
  if(inactivityTimer.getElapsedTime() > 2000) {
    Leds::setColorForAll(Leds::White);
    inactivityTimer.reset();
  } else if(inactivityTimer.getElapsedTime() > 1000) {
    Leds::setColorForAll(Leds::Green);
  }

  Timer::delayMiliseconds(100);
}
