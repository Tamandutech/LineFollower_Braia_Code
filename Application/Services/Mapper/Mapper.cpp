/*
 * Mapper.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "Mapper.hpp"
#include "../../Context/GlobalData.hpp"
#include "../../Drivers/EncoderDriver/EncoderDriver.hpp"
#include "../../Drivers/MotorDriver/MotorDriver.hpp"
#include "../../Drivers/VacuumDriver/VacuumDriver.hpp"
#include "../../Utils/Timer/Timer.hpp"

// TODO
#include "../BLEListener/ble_listener.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
Logger *Mapper::logger = new Logger("Mapper", true, Logger::Level::Info);

void Mapper::readLateral() {
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

    globalData.mapData.push_back({EncoderDriver::getAverage(), calculatePWM()});

    logger->info("#%03d Encoders: %07d", globalData.markCount.load(),
                 EncoderDriver::getAverage());
  }
  // Reading a right mark for the first time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          firstTimeRight) {
    qtdRightMark++;
    readRightBefore = true;
    firstTimeRight  = false;
    EncoderDriver::reset();

    globalData.mapData.push_back({EncoderDriver::getAverage(), calculatePWM()});

    logger->info("#%03d Start of the track", globalData.markCount.load());
  }
  // Reading an intersection
  else if(readingLeft && readingRight && !readIntersecBefore) {
    readIntersecBefore = true;
    readRightBefore    = true;
    readLeftBefore     = true;

    logger->info("Passed through an intersection");
  }
  // Reading a right mark for the second time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          !firstTimeRight) {
    qtdRightMark++;
    globalData.markCount++;

    globalData.mapData.push_back({EncoderDriver::getAverage(), calculatePWM()});

    // TODO testing
    MotorDriver::stop();
    VacuumDriver::stopAfter(700);

    logger->info("#%03d End of the track", globalData.markCount.load());
  }
  // Reading no marks
  else if(notReadingLeft && notReadingRight) {
    readIntersecBefore = false;
    readRightBefore    = false;
    readLeftBefore     = false;
  }
}

// TODO
float Mapper::calculatePWM() { return 100; }

void Mapper::map() {
  uint32_t lastTime = 0;

  // Reset variables
  qtdLeftMark          = 0;
  qtdRightMark         = 0;
  readRightBefore      = false;
  readLeftBefore       = false;
  readIntersecBefore   = false;
  firstTimeRight       = true;
  globalData.markCount = 0;

  logger->info("Waiting to map...");

  // TODO: local loop just for tests
  bool triggered = false;
  while(1) {
    if(run) {
      if(Timer::getMicroseconds() - lastTime >= 1000) {
        MotorDriver::speedOutput(75);
        VacuumDriver::pwmOutput(150);
        readLateral();

        if(qtdRightMark >= 2) break;

        lastTime  = Timer::getMicroseconds();
        triggered = true;
      }
    } else {
      MotorDriver::stop();
      VacuumDriver::pwmOutput(0);

      if(triggered) break;
    }
  }

  logMap();
}

void Mapper::logMap() {
  // #mmm eeeeeee ssss'\n''\0' == 19 characters per line
  const size_t lineLength = 19;
  const char  *header     = "\nMARK ENCODER PWM\n";
  size_t logSize = globalData.markCount.load() * lineLength + strlen(header);
  char   formattingLine[lineLength];
  char  *logMessage = (char *)malloc(logSize);

  // Add the header to the log string
  snprintf(logMessage, logSize, header);

  // Concatenate each line of the mapping
  for(uint8_t m = 0; m <= globalData.markCount.load(); m++) {
    snprintf(formattingLine, lineLength, "#%03d %07d %04.0f\n", m,
             globalData.mapData[m].encoderAverage,
             globalData.mapData[m].baseMotorPWM);

    strncat(logMessage, formattingLine, logSize);
  }

  // TODO remove these delays after implementing a communication task
  Timer::delayMiliseconds(30);
  logger->info("%s", logMessage);
  Timer::delayMiliseconds(500);

  free(logMessage);
  logMessage = NULL;
}