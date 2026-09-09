/*
 * LaboratoryTesting.cpp
 *
 *  Created on: May 19, 2026
 *      Author: Kelvin Novais
 */

#include "LaboratoryTesting.hpp"

#include <cstddef>

#include "../../Drivers/Encoders/Encoders.hpp"
#include "../../Drivers/IMU/IMU.hpp"
#include "../../Drivers/IRSensors/IRSensors.hpp"
#include "../../Drivers/Leds/Leds.hpp"
#include "../../Drivers/Vacuum/Vacuum.hpp"
#include "../../Services/BLE/BLE.hpp"
#include "../../Utils/Battery/Battery.hpp"
#include "../../Utils/DataManager/DataManager.h"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

static void waitForCharacter(const char c) {
  while(BLE::lastCharacter != c) {
    Timer::delayMiliseconds(100);
  }
}

static void testEncoders() {
  Logger::log("Testing Encoders. Send 's' to stop.");

  Encoders::reset();
  while(BLE::lastCharacter != 's') {
    Encoders::update();

    Logger::log("[L: %06ld - R: %06ld]\n"
                "Average: %06ld\n",

                // Individual
                Encoders::counter[Left], Encoders::counter[Right], // NOLINT
                // Average
                Encoders::average); // NOLINT

    Timer::delayMiliseconds(200);
  }

  initializeTests();
}

static void testIMU() {
  // NOLINTBEGIN
  uint32_t currentTime_us = 0;
  uint32_t loggerTimer_ms = Timer::getMiliseconds();
  uint32_t lastTime_us    = Timer::getMicroseconds();
  // NOLINTEND

  Logger::log(
      "Testing IMU...\n"
      "First let's calibrate it: place the robot, and send 'c' to continue.");

  waitForCharacter('c');
  Logger::log("Calibrating...");
  IMU::calibrate();
  Logger::log("Done! Send 's' to stop.");
  IMU::reset();

  while(BLE::lastCharacter != 's') {
    currentTime_us = Timer::getMicroseconds();

    IMU::update(lastTime_us - currentTime_us);

    // Log every 300 ms
    if((Timer::getMiliseconds() - loggerTimer_ms) >= 300) {
      Logger::log(
          // Acceleration
          "aᵢ [m/s²]: (%.2f %.2f %.2f)\n"
          // Speed
          "vᵢ [m/s]: (%.2f %.2f %.2f)\n"
          // Position
          "xᵢ [m]  : (%.2f %.2f %.2f)\n"
          // Angular rate
          "ωᵢ [°/s]: (%.2f %.2f %.2f)\n"
          // Angle
          "θᵢ [°]  : (%.2f %.2f %.2f)\n\n",

          // Acceleration
          IMU::acceleration[X], IMU::acceleration[Y], IMU::acceleration[Z],
          // Speed
          IMU::speed[X], IMU::speed[Y], IMU::speed[Z],
          // Position
          IMU::position[X], IMU::position[Y], IMU::position[Z],
          // Angular rate
          IMU::angularRate[Pitch], IMU::angularRate[Row], IMU::angularRate[Yaw],
          // Angle
          IMU::angle[Pitch], IMU::angle[Row], IMU::angle[Yaw]);

      loggerTimer_ms = Timer::getMiliseconds();
    }

    lastTime_us = currentTime_us;
  }

  initializeTests();
}

static void testIRSensors() {
  uint32_t loggerTimmer = 0;

  Logger::log("Testing IRSensors...\n"
              "First let's calibrate it: send 'c' to continue");

  waitForCharacter('c');
  Logger::log("Calibrating...");
  IRSensors::calibrate();
  Logger::log("Done!\n"
              "Send 's' to stop.");

  while(BLE::lastCharacter != 's') {
    IRSensors::update();

    if(Timer::getMiliseconds() - loggerTimmer >= 200) {
      Logger::log(
          // (I) Sensors
          //     Header - sensors
          "  L1   L2  |  01   02   03   04   05    06  07   08   09   10 "
          "  11   12  |  R1   R2\n"
          //     Values (sensors)
          "[%04u %04u | %04u %04u %04u %04u %04u %04u %04u %04u %04u "
          "%04u %04u %04u | %04u %04u]\n\n"

          // (II) State variables
          "Is on line: %d\n"
          "Is on cross: %d\n"
          "Marks [L,R]: [%d,%d]\n\n",

          // (I) Sensors
          IRSensors::sensorValues[0], IRSensors::sensorValues[1],
          IRSensors::sensorValues[2], IRSensors::sensorValues[3],
          IRSensors::sensorValues[4], IRSensors::sensorValues[5],
          IRSensors::sensorValues[6], IRSensors::sensorValues[7],
          IRSensors::sensorValues[8], IRSensors::sensorValues[9],
          IRSensors::sensorValues[10], IRSensors::sensorValues[11],
          IRSensors::sensorValues[12], IRSensors::sensorValues[13],
          IRSensors::sensorValues[14], IRSensors::sensorValues[15],

          // (II) State variables
          IRSensors::isOnLine, IRSensors::isOnCross, IRSensors::mark[Left],
          IRSensors::mark[Right]);

      loggerTimmer = Timer::getMiliseconds();
    }
  }

  initializeTests();
}

static void testLeds() {
  ColorIndex        index       = FirstRotatableColor;
  Leds::WavingColor wavingColor = {0, 64, 128};

  Logger::log("Testing Leds.");

  // (I) Test predefined colors
  Timer::delayMiliseconds(100);
  Logger::log("Setting color to white");
  Leds::setColorForAll(White);
  while(index != LastRotatableColor) {
    Logger::log("Setting color to %s", Leds::color[index].name);
    Leds::setColorForAll(Leds::color[index].rgb);
    Timer::delayMiliseconds(1000);
    index = static_cast<ColorIndex>(index + 1);
  }

  // (II) Wave colors
  Logger::log("Waving colors. Send 's' to stop.");
  while(BLE::lastCharacter != 's') {
    Leds::setColorForAll({TRIANGULAR_WAVE(wavingColor.r),
                          TRIANGULAR_WAVE(wavingColor.g),
                          TRIANGULAR_WAVE(wavingColor.b)});
    Timer::delayMiliseconds(100);
  }

  initializeTests();
}

static void testVacuum() {
  Logger::log("Testing Vacuum: going from 100 to 1000, in steps of 100.\n "
              "Send 's' to stop the current test, 'n' to go to next step.");

  for(uint16_t pwm = 100; pwm <= 1000; pwm += 100) {
    // (I) Wait
    waitForCharacter('n');

    // (II) Run
    Logger::log("Setting PWM to %d.", pwm);
    Vacuum::pwmAcceleratedOutput(pwm);
    waitForCharacter('s');

    // (III) Stop
    Vacuum::pwmOutput(0);
  }
}

static void testBattery() {
  Logger::log("Testing Battery. Send 's' to stop.");

  while(BLE::lastCharacter != 's') {
    Logger::log("%.2f V", Battery::getBatteryVoltage());
    Timer::delayMiliseconds(200);
  }
}

static void testTimer() {
  const uint16_t tests_ms[] = {500, 200, 100, 50, 10, 1};
  const uint32_t tests_us[] = {2000, 1000, 500, 200, 100, 50, 10, 1};
  const uint32_t tests_ns[] = {5000, 20000, 10000, 1000, 500, 100, 50};
  uint32_t       start      = 0;
  uint32_t       end        = 0;

  Logger::log("Testing Timer.\n"
              "(I) Miliseconds");
  for(uint32_t d : tests_ms) {
    start = Timer::getMiliseconds();
    Timer::delayMiliseconds(d);
    end = Timer::getMiliseconds();

    Logger::log("[%lu ms]: delayed for %lu ms", d, (end - start)); // NOLINT
    Timer::delayMiliseconds(200);
  }

  Logger::log("\n(II) Microseconds");
  for(uint32_t d : tests_us) {
    start = Timer::getMicroseconds();
    Timer::delayMicroseconds(d);
    end = Timer::getMicroseconds();

    Logger::log("[%lu µs]: delayed for %lu µs", d, (end - start)); // NOLINT
    Timer::delayMiliseconds(200);
  }

  Logger::log("\n(III) Nanoseconds");
  for(uint32_t d : tests_ns) {
    start = Timer::getNanoseconds();
    Timer::delayNanoseconds(d);
    end = Timer::getNanoseconds();

    Logger::log("[%lu ns]: delayed for %lu ns", d, (end - start)); // NOLINT
    Timer::delayMiliseconds(200);
  }
}

static void testMemory() {
#if defined(USE_DATA_MANAGER)
  Mapped  mapped  = {0};
  Mapping mapping = {0};

  // (I) Mapped
  // Write
  pool_reset_to(POOL_MODE_MAPPED);
  Logger::log("Populating memory for Mapped... Capacity is: %d",
              pool_get_capacity());
  for(size_t i = 0; i < pool_get_capacity(); i++) {
    mapped.encoder += 100;
    mapped.motor += 10;
    mapped.vacuum += 5;
    ROTATE_COLOR(mapped.colorIndex);

    pool_push_mapped(&mapped);
  }

  // Read
  Timer::delayMiliseconds(25);
  Logger::log("Reading memory...");
  for(size_t i = 0; i < pool_get_count(); i++) {
    pool_get_mapped_data(i, &mapped);

    Timer::delayMiliseconds(10);
    Logger::log("[%03d] %04d %03d %03d %d", i, mapped.encoder, mapped.motor,
                mapped.vacuum, mapped.colorIndex);
  }

  // (II) Mapping
  // Write
  Timer::delayMiliseconds(50);
  pool_reset_to(POOL_MODE_MAPPING);
  Logger::log("\n\nPopulating memory for Mapping... Capacity is: %d",
              pool_get_capacity());
  for(size_t i = 0; i < pool_get_capacity(); i++) {
    mapping.isLeftMark  = i % 2 ? true : false;
    mapping.isRightMark = i % 10 == 0 ? true : false;
    mapping.rightEncoder += 50;
    mapping.leftEncoder += 100;
    mapping.x += 25.25F;
    mapping.y += 50.75F;
    mapping.omega += 1.10F;
    mapping.timestamp = Timer::getMicroseconds();
    ROTATE_COLOR(mapping.colorIndex);

    pool_push_mapping(&mapping);
  }

  // Read
  Timer::delayMiliseconds(25);
  Logger::log("Reading memory...");
  for(size_t i = 0; i < pool_get_count(); i++) {
    if(pool_get_mapping_data(i, &mapping) != POOL_STATUS_OK) {
      Logger::log("Error while fetching Mapping data");
      break;
    }

    Timer::delayMiliseconds(10);
    Logger::log("[%03d] %d %d %03d %03d %.2f %.2f %.2f %d %d", i,
                mapping.isLeftMark, mapping.isRightMark, mapping.rightEncoder,
                mapping.leftEncoder, mapping.x, mapping.y, mapping.omega,
                mapping.colorIndex, mapping.timestamp);
  }
#else
  Logger::log("No custom memory management...");
#endif /* USE_DATA_MANAGER */
}

#if defined(TEST_HARD_FAULTS)
#warning "Tests for hard faults is enabled, be careful!"

static void testHardfaults() {
  Logger::log("Choose a hard fault test. Send:\n"
              "[1] Division by 0\n"
              "[2] Dereference NULL pointer\n"
              "[3] Illegal instruction\n"
              "[4] Read from bad address\n"
              "[5] Write to bad address\n"
              "[q] Quit\n");

  while(BLE::lastCharacter != 'q') {
    BLE::resetLastCharacter();
    Timer::delayMiliseconds(200);

    switch(BLE::lastCharacter) {
    case '1': {
      uint8_t      zero   = 0;
      volatile int result = 1 / zero;
      Logger::log("Dividing by 0...\n"
                  "Got : 1 / % d = % d ",
                  zero, result);

      break;
    }

    case '2': {
      int *null_ptr = NULL;
      Logger::log("Dereferencing NULL pointer...\n"
                  "Got: %d",
                  *null_ptr);

      break;
    }

    case '3': {
      int (*badInstruction)(void) = (int (*)())0xE0000000;
      Logger::log("Executing illegal instruction...\n"
                  "Got %d",
                  badInstruction());

      break;
    }

    case '4': {
      volatile uint32_t *badAddress = (volatile uint32_t *)0xbadcafe;

      Logger::log("Reading from bad address...\n"
                  "Got %ld",
                  *badAddress); // NOLINT

      break;
    }

    case '5': {
      volatile uint64_t *buf = (volatile uint64_t *)0x30000000;
      Logger::log("Writing to bad address...");
      *buf = 0x1122334455667788;

      break;
    }

    default: break;
    }
  }

  // If a handler wasn't called, code will fall here
  Timer::delayMiliseconds(100);
  Logger::log("Fault handler wasn't triggered!");
}
#endif /* TEST_HARD_FAULTS */

void initializeTests() {
  Timer::delayMiliseconds(1500);

  Logger::log("Entering test utility. Send:\n"
              "[1] Encoders\n"
              "[2] IMU\n"
              "[3] IRSensors\n"
              "[4] Leds\n"
              "[5] Vacuum\n"
              "[6] Battery\n"
              "[7] Timer\n"
              "[8] Memory\n"

#if defined(TEST_HARD_FAULTS)
              "[f] Hard faults\n"
#endif /* TEST_HARD_FAULTS */

              "[q] Quit\n");

  while(BLE::lastCharacter != 'q') {
    BLE::resetLastCharacter();
    Timer::delayMiliseconds(200);

    switch(BLE::lastCharacter) {
    case '1': testEncoders(); break;

    case '2': testIMU(); break;

    case '3': testIRSensors(); break;

    case '4': testLeds(); break;

    case '5': testVacuum(); break;

    case '6': testBattery(); break;

    case '7': testTimer(); break;

    case '8': testMemory(); break;

#if defined(TEST_HARD_FAULTS)
    case 'f': testHardfaults(); break;
#endif /* TEST_HARD_FAULTS */

    default: break;
    }
  }
}


/*
 * Hard Fauts Injection:
 * https://interrupt.memfault.com/blog/cortex-m-hardfault-debug
 */
