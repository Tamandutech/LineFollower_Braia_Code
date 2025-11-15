/*
 * Mapper.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "Mapper.hpp"
#include "../../Drivers/EncoderDriver/EncoderDriver.hpp"
#include "../../Drivers/MotorDriver/MotorDriver.hpp"
#include "../../Drivers/VacuumDriver/VacuumDriver.hpp"
#include "../../Utils/Timer/Timer.hpp"

// TODO
#include "../BLEListener/ble_listener.h"

#include <cstdint>

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
uint8_t Mapper::n_marks            = 0;
Logger *Mapper::logger = new Logger("Mapper", true, Logger::Level::Info);

void Mapper::readLateral() {
  QTRSensorDriver::readCalibrated();

  bool readingLeft = QTRSensorDriver::sensorValues[QTRSensorDriver::L_1] <
                         READING_LINE_THRESHOLD ||
                     QTRSensorDriver::sensorValues[QTRSensorDriver::L_2] <
                         READING_LINE_THRESHOLD;

  bool readingRight = QTRSensorDriver::sensorValues[QTRSensorDriver::R_1] <
                          READING_LINE_THRESHOLD ||
                      QTRSensorDriver::sensorValues[QTRSensorDriver::R_2] <
                          READING_LINE_THRESHOLD;

  bool notReadingLeft = QTRSensorDriver::sensorValues[QTRSensorDriver::L_1] >
                            NOT_READING_LINE_THRESHOLD &&
                        QTRSensorDriver::sensorValues[QTRSensorDriver::L_2] >
                            NOT_READING_LINE_THRESHOLD;

  bool notReadingRight = QTRSensorDriver::sensorValues[QTRSensorDriver::R_1] >
                             NOT_READING_LINE_THRESHOLD &&
                         QTRSensorDriver::sensorValues[QTRSensorDriver::R_2] >
                             NOT_READING_LINE_THRESHOLD;

  logger->debug("[%03d %03d] [%03d %03d]",
                QTRSensorDriver::sensorValues[QTRSensorDriver::L_1],
                QTRSensorDriver::sensorValues[QTRSensorDriver::L_2],
                QTRSensorDriver::sensorValues[QTRSensorDriver::R_1],
                QTRSensorDriver::sensorValues[QTRSensorDriver::R_2]);

  // Reading a left mark
  if(readingLeft && notReadingRight && !readLeftBefore) {
    qtdLeftMark++;
    readLeftBefore = true;
    logger->info("#%02d Encoders: %d", n_marks++, EncoderDriver::getAverage());
  }
  // Reading a right mark for the first time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          firstTimeRight) {
    EncoderDriver::reset();
    qtdRightMark++;
    readRightBefore = true;
    firstTimeRight  = false;
    logger->info("Start of the track");
  }
  // Reading a right mark for the second time
  else if(notReadingLeft && readingRight && !readRightBefore &&
          !firstTimeRight) {
    logger->info("End of the track");

    // TODO
    // MotorDriver::stop();
    // VacuumDriver::stopAfter(700);
  }
  // Reading an intersection
  else if(readingLeft && readingRight && !readIntersecBefore) {
    readIntersecBefore = true;
    readRightBefore    = true;
    readLeftBefore     = true;

    logger->info("Passed through an intersection");
  }
  // Reading no marks
  else if(notReadingLeft && notReadingRight) {
    readIntersecBefore = false;
    readRightBefore    = false;
    readLeftBefore     = false;
  }
}

void Mapper::map() {
  uint32_t lastTime = 0;

  // Reset variables
  qtdLeftMark        = 0;
  qtdRightMark       = 0;
  readRightBefore    = false;
  readLeftBefore     = false;
  readIntersecBefore = false;
  firstTimeRight     = true;
  n_marks            = 0;

  logger->info("Waiting to map...");
  Timer::delayMiliseconds(2000);
  EncoderDriver::reset();

  // TODO: local loop just for tests
  bool triggered = false;
  while(1) {
    if(run) {
      if(Timer::getMicroseconds() - lastTime >= 1000) {
        MotorDriver::speedOutput(75);
        VacuumDriver::pwmOutput(150);
        readLateral();

        lastTime  = Timer::getMicroseconds();
        triggered = true;
      }
    } else {
      MotorDriver::stop();
      VacuumDriver::pwmOutput(0);

      if(triggered) break;
    }
  }
}