/*
 * robot.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

// Base header
#include "robot.h"

// Headers from our code base
#include "Context/GlobalData.hpp"
#define EXPOSE_GLOBAL_PERIPHERALS
#include "Context/PeripheralsEnv.hpp"
#include "Context/RobotEnv.hpp"

#include "Services/BLE/BLE.hpp"
#include "Services/Mapper/Mapper.hpp"
#include "Services/PID/PID.hpp"

#include "Utils/Battery/Battery.hpp"
#include "Utils/LaboratoryTesting/LaboratoryTesting.hpp"
#include "Utils/Logger/Logger.hpp"
#include "Utils/Timer/Timer.hpp"

#include "Drivers/Encoders/Encoders.hpp"
#include "Drivers/IMU/IMU.hpp"
#include "Drivers/IRSensors/IRSensors.hpp"
#include "Drivers/Leds/Leds.hpp"
#include "Drivers/Motors/Motors.hpp"
#include "Drivers/Vacuum/Vacuum.hpp"

/*
 * Here we declare private (aka static) variables to this file, but they are
 * still shared between functions
 */
static Logger           *logger = new Logger("Main", false, Logger::Level::All);
static Leds::WavingColor wavingColor = {0, 64, 128};

/*
 * Here we have private (aka static) functions to this file
 */
static void stopRunning() {
  Motors::stop();
  Vacuum::pwmOutput(0);
}

static void startRunning() {
  float    u        = 0;
  uint8_t  i        = 1;
  uint32_t lastTime = 0;

  // Assert
  if(globalData.map.size() < 3) {
    logger->error("Map is too small: %d points", globalData.map.size());
    return;
  }

  logger->info("Running...");

  // Prepare
  Leds::setColorForAll(Magenta);
  Vacuum::pwmAcceleratedOutput(VACUUM_BASE_PWM);
  Encoders::reset();
  lastTime = Timer::getMicroseconds();

  // Start
  while(globalData.action == Action::Run && i < globalData.map.size()) {
    if(Timer::getMicroseconds() - lastTime >= BASE_LOOP_TIME_US) {
      Encoders::update();
      IRSensors::update();

      u = PID::evaluate(IRSensors::error);

      if(Encoders::average >= globalData.map[i - 1].encoderAverage &&
         Encoders::average < globalData.map[i].encoderAverage) {
        // At index i-1
        Motors::pwmOutputFor(Left, globalData.map[i - 1].baseMotorPWM + u);
        Motors::pwmOutputFor(Right, globalData.map[i - 1].baseMotorPWM - u);
        Vacuum::pwmOutput(globalData.map[i - 1].baseVacuumPWM);
        Leds::setColorForAll(globalData.map[i - 1].colorIndex);
      } else if(Encoders::average >= globalData.map[i].encoderAverage) {
        // At index i
        Motors::pwmOutputFor(Left, globalData.map[i].baseMotorPWM + u);
        Motors::pwmOutputFor(Right, globalData.map[i].baseMotorPWM - u);
        Vacuum::pwmOutput(globalData.map[i].baseVacuumPWM);
        Leds::setColorForAll(globalData.map[i].colorIndex);

        logger->info("[%02d] Encoder: %04ld, motor: %03.0f, vacuum: %03.0f", i,
                     Encoders::average, // NOLINT
                     globalData.map[i].baseMotorPWM,
                     globalData.map[i].baseVacuumPWM);

        i++;
      }

      lastTime = Timer::getMicroseconds();
    }
  }

  // Stop graceffuly
  // This action will be performed only if the stop command wasn't sent
  if(globalData.action == Action::Run) {
    Motors::stop();
    Encoders::update();
    logger->info("Stopping at encoder %04ld",
                 Encoders::average); // NOLINT
    Vacuum::stopAfter(2000);
  }
}

// static void startRunningPolling() {
//   // bool isEnabled = true;

//   // uint32_t startTime    = Timer::getMicroseconds();
//   // uint32_t currentTime  = startTime;
//   // uint32_t outStartTime = startTime;

//   // // Prepare
//   // Leds::setColorForAll(Leds::Magenta);
//   // Vacuum::pwmAcceleratedOutput(VACUUM_BASE_PWM);
//   // Encoders::reset();

//   // while (globalData.action == Action::Run) {
//   //   // After Update():
//   //   /*
//   //   1. position
//   //   2. error
//   //   3. isOnLine
//   //   4. lateralMark
//   //   */
//   // }
// }

void setup(void) {
  /*
   * Uncomment the next line to run the BLE module setup (change the name, pin
   * and set baud rate).
   *
   * DO NOT CONNECT to the module while running the setup, otherwise, it won't
   * work
   */
  // BLE::setup();

  // Logger settings
  Logger::setShowTimestamp(false);
  Logger::setShowLogLevel(true);
  Logger::setUseDoubleBreak(false);

  Logger::log("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
              "\nLast build: %s @ %s\n"
              "Robot is starting...\n",
              __DATE__, __TIME__);

  // Initialize base timer
  HAL_TIM_Base_Start(PeripheralsEnv::BASE_TIMER);

  // Initialize drivers
  Motors::initialize();
  Vacuum::initialize();
  Encoders::initialize();

  // Initialize ADC (Needed for IRSensors and Battery)
  HAL_ADCEx_Calibration_Start(PeripheralsEnv::ADC_1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(PeripheralsEnv::ADC_2, ADC_SINGLE_ENDED);
  Timer::delayMiliseconds(100);

  // Initialize DMA (Needed for IRSensors and Battery)
  HAL_ADC_Start_DMA(PeripheralsEnv::ADC_1, (uint32_t *)adc1_buffer,
                    ADC_BUFFER_SIZE);
  HAL_ADC_Start_DMA(PeripheralsEnv::ADC_2, (uint32_t *)adc2_buffer,
                    ADC_BUFFER_SIZE);
  Timer::delayMiliseconds(50);

  // Start DMA reception
  BLE::initialize();

  // Print battery information
  Timer::delayMiliseconds(75);
  if(Battery::getBatteryVoltage() < 7.8F) {
    logger->warning("Low battery: %.2f V", Battery::getBatteryVoltage());
  } else {
    logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  }

  // Initialize IMU
  IMU::initialize();

  Timer::delayMiliseconds(75);
  logger->info("Robot started!");

  // Reset LEDs
  Leds::setColorForAll(White);

  // Testing utility
  Timer::delayMiliseconds(75);
  logger->info("Send 't' to enter the tests");
  for(uint8_t i = 3; i > 0; i--) {
    Timer::delayMiliseconds(1000);
    Logger::log("%d", i);
    if(BLE::lastCharacter == 't') {
      initializeTests();
      break;
    }
  }
  // Reset action due to possible previous interaction
  globalData.action = Action::None;

  // Calibrate sensors
  Timer::delayMiliseconds(75);
  logger->info("Send 'c' to calibrate sensors...");
  BLE::resetLastCharacter();
  while(BLE::lastCharacter != 'c') {
    Timer::delayMiliseconds(100);
  }
  IRSensors::calibrate();

  Timer::delayMiliseconds(25);
  logger->info("Waiting for run command...");

  ///////////////////////////// Manual mapping  ////////////////////////////////
  // Reset variables
  globalData.map.clear();
  globalData.markCount = 0;

  // Assign data
  globalData.map = {
      {0,       MOTOR_BASE_PWM, VACUUM_BASE_PWM, White  },

      {300000,  MOTOR_BASE_PWM, VACUUM_BASE_PWM, Green  },

      // Last point should be the end of the track
      {1600000, 0,              270,             Magenta}
  };
  /****************************************************************************/
}

void loop(void) {
  switch(globalData.action) {
  case Action::Run: startRunning(); break;

  case Action::Map: Mapper::map(); break;

  case Action::None:
  default: stopRunning();
  }

  // Being pedantic to avoid that the robot runs when it shouldn't
  stopRunning();

  // Reset the action variable after the command was performed
  globalData.action = Action::None;

  Leds::setColorForAll({TRIANGULAR_WAVE(wavingColor.r),
                        TRIANGULAR_WAVE(wavingColor.g),
                        TRIANGULAR_WAVE(wavingColor.b)});

  Timer::delayMiliseconds(50);
}
