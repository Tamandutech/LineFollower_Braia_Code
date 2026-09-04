/*
 * robot.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

// Base header
#include "robot.h"

// HAL headers
#include "adc.h"
#include "tim.h"

// Headers from our code base
#include "Context/GlobalData.hpp"
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
  /*
   * [!] ATTENTION [!]
   *
   * Time is measured in MICROseconds here
   */
  uint32_t startTime    = 0;
  uint32_t currentTime  = 0;
  uint32_t lastTime     = 0;
  uint32_t outStartTime = 0;

  uint8_t i = 1;
  float   u = 0;

  bool outWarning      = false;
  bool previousMark[2] = {false};

  // ASSERT MAP SIZE
  if(globalData.map.size() < 3) {
    logger->error("Map is too small: %d points", globalData.map.size());
    return;
  }

  logger->info("Running...");

  // PREPARE
  Leds::setColorForAll(Magenta);
  Vacuum::pwmAcceleratedOutput(VACUUM_BASE_PWM);
  Encoders::reset();
  startTime = currentTime = lastTime = Timer::getMicroseconds();

  // START
  while(globalData.action == Action::Run && i < globalData.map.size()) {
    currentTime = Timer::getMicroseconds();

    if(currentTime - lastTime >= BASE_LOOP_TIME_US) {
      // UPDATE ROBOT STATE
      Encoders::update();
      IRSensors::update();

      // CONTROL SIGNAL
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

      // OUT METHOD
      if(IRSensors::isOnLine) {
        // Reset warning
        outWarning = false;
      } else if(!IRSensors::isOnLine && !outWarning) {
        // Register warning, and save time
        outStartTime = currentTime;
        outWarning   = true;
      } else if(outWarning && (lastTime - outStartTime) >= MAX_OUT_TIME_US) {
        // Stop is out of line for MAX_OUT_TIME_US
        logger->info("Stopped: out of line for %lu µs",
                     lastTime - outStartTime); // NOLINT

        // Comment the 2 following lines to disable out method
        globalData.action = Action::None;
        break;
      }

      // RIGHT MARK
      if(IRSensors::mark[Right] && !previousMark[Right]) {
        // Start of the track
        logger->info("Start of the track");
        previousMark[Right] = true;

        // Uncoment the folowing if you want to set encoder to 0 at the start
        // Encoders::reset();

        Leds::setColorForAll(Green);
        Leds::setColorFor(CenterLed, White);
        Leds::setColorFor(MainBoardLed, White);
      } else if(IRSensors::mark[Right] && previousMark[Right] &&
                (currentTime - startTime) > MIN_TRACK_TIME) {
        // End of the track
        logger->info("End of the track");

        Leds::setColorForAll(Black);
        Leds::setColorFor(CenterLed, White);
        Leds::setColorFor(MainBoardLed, White);

        // Uncoment the following line to stop the robot if out of line
        // break;
      }

      lastTime = Timer::getMicroseconds();
    }
  }

  /*
   * STOP GRACEFULLY
   * This action will be performed only if the stop command wasn't sent or if
   * the robot didn't get out of line.
   */
  if(globalData.action == Action::Run) {
    Motors::stop();
    Encoders::update();
    logger->info("Stopping at encoder %04ld",
                 Encoders::average); // NOLINT
    Vacuum::stopAfter(2000);
  }
}

static void menu() {
  int changeSpeed  = 0;
  int changeVacuum = 0;

  Logger::logSync("Choose an option:\n"
                  "[1] INcrease SPEED by 50\n"
                  "[2] INcrease SPEED by 100\n"
                  "[3] DEcrease SPEED by 50\n"
                  "[4] DEcrease SPEED by 100\n"
                  "-------------------------"
                  "[5] INcrease VACUUM by 50\n"
                  "[6] INcrease VACUUM by 100\n"
                  "[7] DEcrease VACUUM by 50\n"
                  "[8] DEcrease VACUUM by 100\n"
                  "[9] Quit\n");

  BLE::resetLastCharacter();
  while(BLE::lastCharacter != 'q') {
    BLE::resetLastCharacter();
    Timer::delayMiliseconds(200);

    switch(BLE::lastCharacter) {
    case '1': changeSpeed = +50; break;

    case '2': changeSpeed = +100; break;

    case '3': changeSpeed = -50; break;

    case '4': changeSpeed = -100; break;

    case '5': changeVacuum = +50; break;

    case '6': changeVacuum = +100; break;

    case '7': changeVacuum = -50; break;

    case '8': changeVacuum = -100; break;

    default: break;
    }
  }

  for(int i = 0; i < globalData.map.size() - 1; i++) {
    globalData.map[i].baseMotorPWM += changeSpeed;
    globalData.map[i].baseVacuumPWM += changeVacuum;
  }

  Logger::logSync("Added %03d to speed\n"
                  "Added %03d to vacuum\n",
                  changeSpeed, changeVacuum);

  BLE::resetLastCharacter();
}

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
  HAL_TIM_Base_Start(&BASE_TIMER);

  // Initialize ADC (Needed for IRSensors and Battery)
  HAL_ADCEx_Calibration_Start(&ADC_1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&ADC_2, ADC_SINGLE_ENDED);
  Timer::delayMiliseconds(100);

  // Initialize DMA (Needed for IRSensors and Battery)
  HAL_ADC_Start_DMA(&ADC_1, (uint32_t *)adc1_buffer, ADC_BUFFER_SIZE);
  HAL_ADC_Start_DMA(&ADC_2, (uint32_t *)adc2_buffer, ADC_BUFFER_SIZE);
  Timer::delayMiliseconds(50);

  // Initialize drivers
  Motors::initialize();
  Vacuum::initialize();
  Encoders::initialize();
  IRSensors::initialize();
  BLE::initialize();
  IMU::initialize();

  // Print battery information
  Timer::delayMiliseconds(75);
  if(Battery::getBatteryVoltage() < 7.8F) {
    logger->warning("Low battery: %.2f V", Battery::getBatteryVoltage());
  } else {
    logger->info("Battery voltage: %.2f V", Battery::getBatteryVoltage());
  }

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
      {1600000, 0,              270,             Magenta},
  };
  /****************************************************************************/
}

void loop(void) {
  switch(globalData.action) {
  case Action::Run: startRunning(); break;

  case Action::Map: Mapper::map(); break;

  case Action::Menu: menu(); break;

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
