/*
 * Mapper.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "../../../Robot/Services/Mapper/Mapper.hpp"

#include "../../Utils/Timer/Timer.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../../Robot/Context/GlobalData.hpp"
#include "../../../Robot/Context/RobotEnv.hpp"
#include "../../Drivers/Encoders/Encoders.hpp"
#include "../../Drivers/Leds/Leds.hpp"
#include "../../Drivers/Motors/Motors.hpp"
#include "../../Drivers/Vacuum/Vacuum.hpp"
#include "../BLE/BLE.hpp"

// Returns ~1000 if reading BLACK
// Returns ~0    if reading WHITE
#define NOT_READING_LINE_THRESHOLD 800
#define READING_LINE_THRESHOLD     200

uint8_t Mapper::qtdLeftMark        = 0;
uint8_t Mapper::qtdRightMark       = 0;
bool    Mapper::readRightBefore    = false;
bool    Mapper::readLeftBefore     = false;
bool    Mapper::readIntersecBefore = false;
bool    Mapper::firstTimeRight     = true;
Logger *Mapper::logger = new Logger("Mapper", false, Logger::Level::Info);

static const int           N_COLORS        = 8;
static Leds::RgbColor color[N_COLORS] = {
    Leds::Colors.red,    Leds::Colors.blue,
    Leds::Colors.green,  Leds::Colors.magenta,
    Leds::Colors.yellow, Leds::Colors.indigo,
    Leds::Colors.orange, Leds::Colors.cyan};

static const char *colorName[N_COLORS] = {
    "red", "blue", "green", "magenta", "yellow", "indigo", "orange", "cyan"};

void Mapper::readLateral() {
  static int colorIndex = 0;

  readCalibrated();

  bool readingLeft = sensorValues[L_1] < READING_LINE_THRESHOLD ||
                     sensorValues[L_2] < READING_LINE_THRESHOLD;

  bool readingRight = sensorValues[R_1] < READING_LINE_THRESHOLD ||
                      sensorValues[R_2] < READING_LINE_THRESHOLD;

  bool notReadingLeft = sensorValues[L_1] > NOT_READING_LINE_THRESHOLD &&
                        sensorValues[L_2] > NOT_READING_LINE_THRESHOLD;

  bool notReadingRight = sensorValues[R_1] > NOT_READING_LINE_THRESHOLD &&
                         sensorValues[R_2] > NOT_READING_LINE_THRESHOLD;

  logger->debug("[%03d %03d] [%03d %03d]", sensorValues[L_1], sensorValues[L_2],
                sensorValues[R_1], sensorValues[R_2]);

  // Reading a left mark
  if(readingLeft && notReadingRight && !readLeftBefore) {
    globalData.markCount++;
    qtdLeftMark++;
    readLeftBefore = true;

    globalData.mapData.push_back({Encoders::getAverage(), calculatePWM(),
                                  RobotEnv::VACUUM_BASE_PWM,
                                  Leds::Colors.white});

    colorIndex = (colorIndex + 1) % N_COLORS;
    Leds::setColorForAll(color[colorIndex]);

    logger->info("#%03d Encoders: %07ld %s", globalData.markCount.load(),
                 Encoders::getAverage(), colorName[colorIndex]); // NOLINT
  }
  // Reading a right mark for the first time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          firstTimeRight) {
    qtdRightMark++;
    readRightBefore = true;
    firstTimeRight  = false;

    globalData.mapData.push_back({Encoders::getAverage(), calculatePWM(),
                                  RobotEnv::VACUUM_BASE_PWM,
                                  Leds::Colors.white});

    logger->info("#%03d Start of the track", globalData.markCount.load());
  }
  // Reading an intersection
  else if(readingLeft && readingRight && !readIntersecBefore) {
    readIntersecBefore = true;
    readRightBefore    = true;
    readLeftBefore     = true;

    logger->info("Intersection");
  }
  // Reading a right mark for the second time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          !firstTimeRight) {
    readRightBefore = true;
    qtdRightMark++;
    globalData.markCount++;

    globalData.mapData.push_back({Encoders::getAverage(), calculatePWM(),
                                  RobotEnv::VACUUM_BASE_PWM,
                                  Leds::Colors.white});

    logger->info("#%03d Encoders: %07ld [Right]", globalData.markCount.load(),
                 Encoders::getAverage()); // NOLINT
  }
  // Reading no marks
  else if(notReadingLeft && notReadingRight) {
    readIntersecBefore = false;
    readRightBefore    = false;
    readLeftBefore     = false;
  }
}

/*
 * TODO
 * This function should calculate the PWM accordingly to the left and right
 * encoders (less speed on short curves, and high speed on straights)...
 */
float Mapper::calculatePWM() { return 100; }

void Mapper::map() {
  uint32_t lastTime = 0;

  // Reset variables
  qtdLeftMark        = 0;
  qtdRightMark       = 0;
  readRightBefore    = false;
  readLeftBefore     = false;
  readIntersecBefore = false;
  firstTimeRight     = true;

  globalData.mapData.clear();
  globalData.markCount = 0;

  Leds::setColorForAll(Leds::Colors.magenta);
  logger->info("Mapping...");
  Encoders::reset();

  Vacuum::pwmAcceleratedOutput(RobotEnv::MOTOR_BASE_PWM);
  lastTime = Timer::getMicroseconds();

  while(BLEListener::action == BLEListener::Map) {
    if(Timer::getMicroseconds() - lastTime >= 1000) {
      Motors::pwmOutput(RobotEnv::MOTOR_BASE_PWM);
      Vacuum::pwmOutput(RobotEnv::VACUUM_BASE_PWM);
      readLateral();

      lastTime = Timer::getMicroseconds();
    }
  }

  Motors::stop();
  Vacuum::stopAfter(1500);

  logMap();
}

void Mapper::logMap() {
  // #mmm eeeeeee ssss'\0' == 18 characters per line
  const size_t lineLength = 18;
  char         line[lineLength];

  logger->info("\nMARK ENCODER PWM\n");
  Timer::delayMiliseconds(20);

  // Send each line of the mapping
  for(size_t m = 0; m < globalData.mapData.size(); m++) {
    int ret;
    ret = snprintf(static_cast<char *>(line), lineLength, "#%03d %07ld %04.0f",
                   m, globalData.mapData.at(m).encoderAverage, // NOLINT
                   globalData.mapData.at(m).baseMotorPWM);

    if(ret < 0) continue;

    Logger::log("%s", static_cast<const char *>(line));
    // TODO remove these delays after implementing a communication task
    Timer::delayMiliseconds(20);
  }

  logger->info("Done!\n");
  Timer::delayMiliseconds(20);
}
