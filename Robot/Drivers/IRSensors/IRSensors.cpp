/*
 * QTRSensorDriver.cpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

#include "IRSensors.hpp"

#include <algorithm>
#include <cstdint>

#include "adc.h"

#include "../../Utils/Timer/Timer.hpp"
#include "../Leds/Leds.hpp"

// The delay between two sensors readings while calibrating
#define DELAY 100

// How many samples we are getting to determine min and max sensors values
#define SAMPLES ((uint16_t)500)

/*
 * Since the ADC buffers are in a messy order, we create an array of pointers
 * to get the correct values in the correct order.
 * We just need to translate manually the addresses.
 *
 * For example, to access the right encoder: *sensorValues[R_1]
 */
uint32_t *const IRSensors::rawSensorValues[_N_SENSORS] = {
    // Left
    [L_1] = &adc2_buffer[5],
    [L_2] = &adc1_buffer[6],

    // Center
    [C_1]  = &adc1_buffer[7],
    [C_2]  = &adc2_buffer[4],
    [C_3]  = &adc2_buffer[2],
    [C_4]  = &adc2_buffer[1],
    [C_5]  = &adc2_buffer[0],
    [C_6]  = &adc2_buffer[6],
    [C_7]  = &adc2_buffer[7],
    [C_8]  = &adc1_buffer[3],
    [C_9]  = &adc1_buffer[2],
    [C_10] = &adc1_buffer[1],
    [C_11] = &adc1_buffer[0],
    [C_12] = &adc2_buffer[3],

    // Right
    [R_1] = &adc1_buffer[5],
    [R_2] = &adc1_buffer[4]};

// Minimum values ​​measured in practice
uint32_t IRSensors::minValues[_N_SENSORS] = {
    // Left
    208, 208,

    // Center
    208, 199, 199, 199, 194, 198, 197, 197, 196, 198, 199, 207,

    // Right
    207, 207};

// Maximum values ​​measured in practice
uint32_t IRSensors::maxValues[_N_SENSORS] = {
    // Left
    3650, 3650,

    // Center
    3661, 3473, 3655, 3523, 3427, 3508, 3493, 3515, 3533, 3481, 3476, 3646,

    // Right
    3645, 3645};

uint16_t                IRSensors::sensorValues[_N_SENSORS] = {0};
bool                    IRSensors::calibrated               = false;
uint16_t                IRSensors::lastPosition             = 0;
uint16_t                IRSensors::arraySensorCenter        = 5500;
const IRSensors::Sensor IRSensors::firstCentralSensor       = C_1;
const IRSensors::Sensor IRSensors::lastCentralSensor        = C_12;
Logger                 *IRSensors::logger =
    new Logger("QTRSensorDriver", false, Logger::Level::Info);

void IRSensors::calibrateSensors() {
  // Reset the values
  for(uint8_t i = 0; i < _N_SENSORS; i++) {
    minValues[i] = 4095;
    maxValues[i] = 0;
  }

  logger->info("Calibrating sensors...");
  calibrated = true;

  Leds::setColorForAll(Leds::Colors.blue);

  for(uint16_t s = 0; s < SAMPLES; s++) {
    // Get the min and max values
    for(uint8_t i = 0; i < _N_SENSORS; i++) {
      minValues[i] = std::min(minValues[i], *rawSensorValues[i]);
      maxValues[i] = std::max(minValues[i], *rawSensorValues[i]);
    }

    // Every 10 samples, ensure that (max > min) and (min < max)
    if((s % 10) == 0) {
      for(uint8_t j = 0; j < _N_SENSORS; j++) {
        maxValues[j] = std::max(minValues[j], maxValues[j]);
        minValues[j] = std::min(maxValues[j], minValues[j]);
      }

      Timer::delayMiliseconds(DELAY);
    }
  }

  /*
   * From the empirical values:
   *    Max value average: 3425
   *    Min value average: 200
   *    Difference is:     3225 (we're going to consider 2300)
   *
   * If the difference between the values for calibration is less than
   * difference, we can consider it unsuccessful
   */
  for(uint8_t i = 0; i < _N_SENSORS; i++) {
    if(maxValues[i] - minValues[i] < 2300) {
      calibrated = false;
      logger->warning(
          "Bad calibration for sensor #%02d\n    ↳ Max/Min: [%04lu, %04lu]", i,
          maxValues[i], minValues[i]); // NOLINT
      Timer::delayMiliseconds(75);
    }
  }

  if(calibrated)
    logger->info("Sensors calibrated");
  else
    logger->error("SENSORS NOT CALIBRATED, REDO IT!");
  Timer::delayMiliseconds(25);

  // NOLINTBEGIN
  logger->debug("Max/Min:\n"
                "[%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%"
                "04lu,%04lu,%"
                "04lu,%04lu,%04lu,%04lu]\n"
                "[%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%04lu,%"
                "04lu,%04lu,%"
                "04lu,%04lu,%04lu,%04lu]",
                // MAX
                //  Left
                maxValues[0], maxValues[1],
                // Center
                maxValues[2], maxValues[3], maxValues[4], maxValues[5],
                maxValues[6], maxValues[7], maxValues[8], maxValues[9],
                maxValues[10], maxValues[11], maxValues[12], maxValues[13],
                // Right
                maxValues[14], maxValues[15],

                // MIN
                //  Left
                minValues[0], minValues[1],
                // Center
                minValues[2], minValues[3], minValues[4], minValues[5],
                minValues[6], minValues[7], minValues[8], minValues[9],
                minValues[10], minValues[11], minValues[12], minValues[13],
                // Right
                minValues[14], minValues[15]);
  // NOLINTEND
}

void IRSensors::readCalibrated() {
  // Emit a warning once, if sensors aren't calibrated
  if(!calibrated) {
    logger->warning("Sensors not calibrated!");
    calibrated = true;
  }

  for(uint8_t i = 0; i < _N_SENSORS; i++) {
    uint16_t calmin, calmax;

    calmax = maxValues[i];
    calmin = minValues[i];

    uint16_t denominator = calmax - calmin;
    int16_t  value       = 0;

    if(denominator != 0)
      value = (((uint16_t)*rawSensorValues[i]) - calmin) * 1000 / denominator;

    if(value < 0)
      value = 0;
    else if(value > 1000)
      value = 1000;

    sensorValues[i] = value;
  }
}

uint16_t IRSensors::readLine() {
  bool     onLine = false;
  uint32_t avg    = 0; // this is for the weighted total
  uint16_t sum    = 0; // this is for the denominator, which is <= 64000

  readCalibrated();

  for(uint8_t i = firstCentralSensor; i <= lastCentralSensor; i++) {
    uint16_t value = sensorValues[i];

    value = 1000 - value;

    // Keep track of whether we see the line at all
    if(value > 200) {
      onLine = true;
    }

    // Only average in values that are above a noise threshold
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

int16_t IRSensors::getError() {
  /*
   * The readLine() returns a value from 0 to 11000; 5500 is the middle of the
   * sensor array.
   *
   * The error is computed according to the defined center value.
   */
  return (readLine() - arraySensorCenter);
}

void IRSensors::setArraySensorCenter(uint16_t center) {
  arraySensorCenter = center;
}
