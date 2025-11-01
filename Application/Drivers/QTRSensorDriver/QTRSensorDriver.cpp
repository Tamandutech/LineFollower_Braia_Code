/*
 * QTRSensorDriver.cpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

#include "QTRSensorDriver.hpp"
#include "../LedDriver/LedDriver.hpp"
#include "../../Utils/Timer.hpp"

// TODO tmp
#include "platform_functions.h"

#include <algorithm>
#include <cstdint>

// The delay between two sensors readings while calibrating
#define DELAY 100

// How many samples we are getting to determine min and max sensors values
#define REPETITIONS ((uint16_t) 500)

extern "C" {
  extern volatile uint32_t adc1_buffer[9];
  extern volatile uint32_t adc2_buffer[9];
};

uint32_t QTRSensorDriver::minValues[_N_SENSORS] = {
  // Left
  208,
  208,

  // Center
  208,
  199,
  199,
  199,
  194,
  198,
  197,
  197,
  196,
  198,
  199,
  207,

  // Right
  207,
  207
};

uint32_t QTRSensorDriver::maxValues[_N_SENSORS] = {
  // Left
  3650,
  3650,

  // Center
  3661,
  3473,
  3655,
  3523,
  3427,
  3508,
  3493,
  3515,
  3533,
  3481,
  3476,
  3646,

  // Right
  3645,
  3645
};

uint32_t QTRSensorDriver::sensorValues[_N_SENSORS] = {0};
bool QTRSensorDriver::calibrated = false;
uint16_t QTRSensorDriver::lastPosition = 0;
const QTRSensorDriver::Sensor QTRSensorDriver::firstCentralSensor = C_1;
const QTRSensorDriver::Sensor QTRSensorDriver::lastCentralSensor = C_12;
Logger *QTRSensorDriver::logger = new Logger("QTRSensorDriver", false,
static_cast<Logger::Level>(Logger::Debug | Logger::Info));

void QTRSensorDriver::updateAdc() {
  // adc1
  sensorValues[12] = adc1_buffer[0];
  sensorValues[11] = adc1_buffer[1];
  sensorValues[10] = adc1_buffer[2];
  sensorValues[9] = adc1_buffer[3];
  sensorValues[15] = adc1_buffer[4];
  sensorValues[14] = adc1_buffer[5];
  sensorValues[1] = adc1_buffer[6];
  sensorValues[2] = adc1_buffer[7];

  // adc2
  sensorValues[6] = adc2_buffer[0];
  sensorValues[5] = adc2_buffer[1];
  sensorValues[4] = adc2_buffer[2];
  sensorValues[13] = adc2_buffer[3];
  sensorValues[3] = adc2_buffer[4];
  sensorValues[0] = adc2_buffer[5];
  sensorValues[7] = adc2_buffer[6];
  sensorValues[8] = adc2_buffer[7];
}

void QTRSensorDriver::calibrateSensors() {
  // Reset the values 
  for (uint8_t i = 0; i < _N_SENSORS; i++) {
    minValues[i] = 4095;
    maxValues[i] = 0;
  }
  
  logger->info("Calibrating sensors...");
  
  LedDriver::setColorForAll(LedDriver::Colors.red);

  for (uint16_t r = 0; r < REPETITIONS; r++) {
    updateAdc();

    for (uint8_t i = 0; i < _N_SENSORS; i++) {
      minValues[i] = std::min(minValues[i], sensorValues[i]);
      maxValues[i] = std::max(minValues[i], sensorValues[i]);
    }

    if ((r % 10) == 0) {
      for (uint8_t j = 0; j < _N_SENSORS; j++) {
        maxValues[j] = std::max(minValues[j], maxValues[j]);
        minValues[j] = std::min(maxValues[j], minValues[j]);
      }

      Timer::delayMiliseconds(DELAY);
    }
  }

  logger->debug(
    "Max/Min:\n"\
    "[%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d]\n"\
    "[%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d,%04d]",
    //MAX
    // Left
    maxValues[0], maxValues[1],
    // Center
    maxValues[2], maxValues[3], maxValues[4], maxValues[5], maxValues[6],
    maxValues[7], maxValues[8], maxValues[9], maxValues[10], maxValues[11],
    maxValues[12], maxValues[13],
    // Right
    maxValues[14], maxValues[15],

    //MIN
    // Left
    minValues[0], minValues[1],
    // Center
    minValues[2], minValues[3], minValues[4], minValues[5], minValues[6],
    minValues[7], minValues[8], minValues[9], minValues[10], minValues[11],
    minValues[12], minValues[13],
    // Right
    minValues[14], minValues[15]
  );

  calibrated = true;

  // logger->info("Sensors calibrated");
  LedDriver::setColorForAll(LedDriver::Colors.black);
}

void QTRSensorDriver::readCalibrated() {
  if(!calibrated) {
    logger->warning("Sensors not calibrated!");
    calibrated = true;
  }

  // read the needed values
  updateAdc();

  for(uint8_t i = firstCentralSensor; i <= lastCentralSensor; i++) {
    uint16_t calmin, calmax;

    calmax = maxValues[i];
    calmin = minValues[i];

    uint16_t denominator = calmax - calmin;
    int16_t  value       = 0;

    if(denominator != 0)
      value = (((uint16_t)sensorValues[i]) - calmin) * 1000 / denominator;

    if(value < 0)
      value = 0;
    else if(value > 1000)
      value = 1000;

    sensorValues[i] = value;
  }
  // logger->debug("%d | %d | %d | %d | %d | %d | %d | %d\n", sensorValues[0],
  //          sensorValues[1], sensorValues[2], sensorValues[3], sensorValues[4],
  //          sensorValues[5], sensorValues[6], sensorValues[7]);
}

uint16_t QTRSensorDriver::readLine() {
  bool     onLine = false;
  uint32_t avg    = 0; // this is for the weighted total
  uint16_t sum    = 0; // this is for the denominator, which is <= 64000

  readCalibrated();

  for(uint8_t i = firstCentralSensor; i <= lastCentralSensor; i++) {
    uint16_t value = sensorValues[i];

    value = 1000 - value;  

    // keep track of whether we see the line at all
    if(value > 200) {
      onLine = true;
    }

    // only average in values that are above a noise threshold
    if(value > 50) {
      avg += (uint32_t)value * ((i - firstCentralSensor) * 1000);
      sum += value;
    }
  }

  if(!onLine) {
    // If it last read to the left of center, return 0.
    if(lastPosition < (lastCentralSensor - firstCentralSensor) * 1000 / 2) {
      return 0;
    }
    // If it last read to the right of center, return the max.
    else {
      return (lastCentralSensor - firstCentralSensor) * 1000;
    }
  }

  lastPosition = avg / sum;
  return lastPosition;
}
