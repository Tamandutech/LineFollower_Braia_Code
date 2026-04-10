/*
 * main.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "application.h"

// Headers from STM HAL
#include "adc.h"
#include "tim.h"

// Headers from our code base
#include "Context/GlobalData.hpp"
#include "Context/RobotEnv.hpp"

#include "Services/BLEListener/BLEListener.hpp"
#include "Services/Mapper/Mapper.hpp"

#include "Utils/Battery/Battery.hpp"
#include "Utils/Logger/Logger.hpp"
#include "Utils/Timer/Timer.hpp"

#include "Drivers/EncoderDriver/EncoderDriver.hpp"
#include "Drivers/LedDriver/LedDriver.hpp"
#include "Drivers/MotorDriver/MotorDriver.hpp"
#include "Drivers/QTRSensorDriver/QTRSensorDriver.hpp"
#include "Drivers/VacuumDriver/VacuumDriver.hpp"

// Standard headers
#include <cstdint>

/*
 * Here we declare private (aka static) variables to this file, but they are
 * still shared between functions
 */
static Logger *logger = new Logger("Main", true, Logger::Level::All);

/*
 * Here we have private (aka static) functions to this file
 */
static void stopRunning() {
  MotorDriver::stop();
  VacuumDriver::stopAfter(700);
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
  LedDriver::setColorForAll(LedDriver::Colors.magenta);
  VacuumDriver::pwmAcceleratedOutput(RobotEnv::BASE_VACUUM_PWM);
  EncoderDriver::reset();
  lastTime = Timer::getMicroseconds();

  // Start
  while(BLEListener::action == BLEListener::Run &&
        i < globalData.mapData.size()) {
    if(Timer::getMicroseconds() - lastTime >= 1000) {
      int32_t avg = EncoderDriver::getAverage();

      if(avg >= globalData.mapData[i - 1].encoderAverage &&
         avg < globalData.mapData[i].encoderAverage) {
        // At index i-1
        MotorDriver::pwmOutput(globalData.mapData[i - 1].baseMotorPWM);
        VacuumDriver::pwmOutput(globalData.mapData[i - 1].baseVacuumPWM);
        LedDriver::setColorForAll(globalData.mapData[i - 1].color);
      } else if(avg >= globalData.mapData[i].encoderAverage) {
        // At index i
        MotorDriver::pwmOutput(globalData.mapData[i].baseMotorPWM);
        VacuumDriver::pwmOutput(globalData.mapData[i].baseVacuumPWM);
        LedDriver::setColorForAll(globalData.mapData[i].color);

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
  if(BLEListener::action == BLEListener::Run) {
    MotorDriver::stop();
    logger->info("Stopping at encoder %04ld",
                 EncoderDriver::getAverage()); // NOLINT
    VacuumDriver::stopAfter(1000);
  }
}

static void customAction() {
  /*
   * Implement here a custom action
   */
}

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
  HAL_ADC_Start_DMA(&hadc1, static_cast<uint32_t *>(adc1_buffer), 9);
  HAL_ADC_Start_DMA(&hadc2, static_cast<uint32_t *>(adc2_buffer), 9);
  HAL_Delay(50);

  // Start DMA reception
  BLEListener::start();

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

  // Logger
  Logger::setShowTimestamp(false);
  Logger::setShowLogLevel(true);
  Logger::setUseDoubleBreak(false);

  // Reset LEDs
  LedDriver::setColorForAll(LedDriver::Colors.black);

  // Calibrate sensors
  Timer::delayMiliseconds(1000);
  // QTRSensorDriver::calibrateSensors();

  // Reset encoders
  EncoderDriver::reset();

  Timer::delayMiliseconds(25);
  logger->info("Waiting for run command...");

  ///////////////////////////// Manual mapping  ////////////////////////////////
  // Reset variables
  globalData.mapData.clear();
  globalData.markCount = 0;

  // Assign data
  globalData.mapData = {
      {0,      85, 250, LedDriver::Colors.red   },

      // Zig zag
      {15000,  85, 250, LedDriver::Colors.blue  },

      // Trombone
      {85000,  85, 270, LedDriver::Colors.green },

      // Straight
      {170000, 80, 270, LedDriver::Colors.red   },

      // Snail
      {300000, 85, 270, LedDriver::Colors.cyan  },

      // Short straight
      {505000, 85, 270, LedDriver::Colors.orange},

      // Squares
      {570000, 85, 270, LedDriver::Colors.indigo},

      // Infinite
      {750000, 85, 270, LedDriver::Colors.blue  },

      // Last point should be the end of the track
      {940000, 0,  270, LedDriver::Colors.white }
  };
  /****************************************************************************/
}

void loop(void) {
  static Timer inactivityTimer(Timer::Miliseconds);

  switch(BLEListener::action) {
  case BLEListener::Run: startRunning(); break;

  case BLEListener::Map: Mapper::map(); break;

  case BLEListener::CustomAction: customAction(); break;

  case BLEListener::None:
  default: stopRunning();
  }

  // Being pedantic to avoid that the robot runs when it shouldn't
  stopRunning();

  // Reset the action variable after the command was performed
  BLEListener::action = BLEListener::None;

  // Blink LEDs to show inactivity
  if(inactivityTimer.getElapsedTime() > 2000) {
    LedDriver::setColorForAll(LedDriver::Colors.white);
    inactivityTimer.reset();
  } else if(inactivityTimer.getElapsedTime() > 1000) {
    LedDriver::setColorForAll(LedDriver::Colors.green);
  }

  Timer::delayMiliseconds(100);
}
