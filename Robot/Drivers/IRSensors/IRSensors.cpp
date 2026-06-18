/*
 * IRSensors.cpp
 *
 *  Created on: Oct 31, 2025
 *      Author: Kelvin Novais
 */

/******************************************************************************/
// INCLUDES

#include "IRSensors.hpp"

#include <algorithm>

#include "adc.h"

#include "../../Utils/Timer/Timer.hpp"
#include "../Leds/Leds.hpp"


/******************************************************************************/
// DEFINES

// (I) Sampling:
// The delay between two sensors readings while calibrating
#define SAMPLING_DELAY 1 // ms

// How many samples we are getting to determine min and max sensors values
#define SAMPLES 2000

// (II) Raw constants
#define MIN_RAW_VALUE 0
#define MAX_RAW_VALUE 4095

#define MIN_EMPIRICAL_RAW_VALUE 200
#define MAX_EMPIRICAL_RAW_VALUE 3550
#define MIN_CALIBRATION_DIFFERENCE \
  ((MAX_EMPIRICAL_RAW_VALUE - MIN_EMPIRICAL_RAW_VALUE) * 0.70)

// (III) Calibrated constants
/*
 * Returns ~1000 if reading WHITE
 * Returns ~0    if reading BLACK
 */
#define MIN_SENSOR_VALUE         0    // ‰
#define SENSOR_ON_LINE_THRESHOLD 600  // ‰
#define MAX_SENSOR_VALUE         1000 // ‰
#define SENSOR_NOISE_THRESHOLD   50   // ‰

// (III) Right mark detection/stop conditioning
// Min time to register a new right mark detection
#define RIGHT_MARK_TIMEOUT      150 // ms
// Min time to wait for the detection of a left mark, without considering the
// final right mark
#define RIGHT_MARK_WAITING_LEFT 100 // ms

// (IV) Position
#define MIN_POSITION 0
#define MAX_POSITION ((LastCentral - FirstCentral) * 1000)
#define CENTRAL_POSITION ((LastCentral - FirstCentral) * 1000 / 2)


/******************************************************************************/
// VARIABLES
// (I) Private
/*
 * Since the ADC buffers are in a messy order, we create an array of pointers
 * to get the correct values in the correct order.
 * We just need to translate manually the addresses.
 *
 * For example, to access the right encoder: *sensorValues[R_1]
 */
const volatile uint32_t *const IRSensors::rawValues[N_SENSORS_] = {
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

#define n MIN_EMPIRICAL_RAW_VALUE
#define X MAX_EMPIRICAL_RAW_VALUE
// Minimum values ​​measured in practice
uint32_t IRSensors::minRawValues_[N_SENSORS_] = {n, n, n, n, n, n, n, n,
                                                 n, n, n, n, n, n, n, n};

// Maximum values ​​measured in practice
uint32_t IRSensors::maxRawValues_[N_SENSORS_] = {X, X, X, X, X, X, X, X,
                                                 X, X, X, X, X, X, X, X};
#undef n
#undef X

Logger *IRSensors::logger = new Logger("IRSensors", false, Logger::Level::Info);
uint32_t IRSensors::markDetecionTime_[N_SIDES_] = {0};
uint16_t IRSensors::sensorValue_[N_SENSORS_]    = {0};
uint16_t IRSensors::previousPosition_           = 0;
uint16_t IRSensors::position_                   = 0;
int16_t  IRSensors::error_                      = 0;
bool     IRSensors::mark_[N_SIDES_]             = {false};
bool     IRSensors::isOnLine_                   = false;
bool     IRSensors::isOnCross_                  = false;
bool     IRSensors::previousMark_[N_SIDES_]     = {false};
bool     IRSensors::calibrated_                 = false;

// (II) Public
const uint16_t (&IRSensors::sensorValues)[N_SENSORS_] = sensorValue_;
const uint16_t &IRSensors::position                   = position_;
const int16_t  &IRSensors::error                      = error_;
const bool (&IRSensors::mark)[N_SIDES_]               = mark_;
const bool &IRSensors::isOnLine                       = isOnLine_;
const bool &IRSensors::isOnCross                      = isOnCross_;


void IRSensors::calibrate() {
  // (I) RESET THE VALUES
  for(uint8_t i = 0; i < N_SENSORS_; i++) {
    minRawValues_[i] = MAX_RAW_VALUE;
    maxRawValues_[i] = MIN_RAW_VALUE;
  }

  logger->info("Calibrating sensors...");
  calibrated_ = true;

  Leds::setColorForAll(Blue);

  // (II) GET MIN AND MAX
  for(uint16_t s = 0; s < SAMPLES; s++) {
    // Get the min and max values
    for(uint8_t i = 0; i < N_SENSORS_; i++) {
      uint32_t rawValue = *rawValues[i];

      minRawValues_[i] = std::min(minRawValues_[i], rawValue);
      maxRawValues_[i] = std::max(maxRawValues_[i], rawValue);
    }

    // Every 10 samples, ensure that (max > min) and (min < max)
    if((s % 10) == 0) {
      for(uint8_t j = 0; j < N_SENSORS_; j++) {
        maxRawValues_[j] = std::max(minRawValues_[j], maxRawValues_[j]);
        minRawValues_[j] = std::min(maxRawValues_[j], minRawValues_[j]);
      }
    }

    Timer::delayMiliseconds(SAMPLING_DELAY);
  }

  // (III) Emit warning for bad calibrations
  /*
   * If the difference between the values for calibration is less than
   * difference, we can consider it unsuccessful
   */
  for(uint8_t i = 0; i < N_SENSORS_; i++) {
    if(maxRawValues_[i] - minRawValues_[i] < MIN_CALIBRATION_DIFFERENCE) {
      calibrated_ = false;
      logger->warning(
          "Bad calibration for sensor #%02d\n    ↳ Max/Min: [%04lu, %04lu]", i,
          maxRawValues_[i], minRawValues_[i]); // NOLINT
      Timer::delayMiliseconds(75);
    }
  }

  if(calibrated_)
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
                // Left
                maxRawValues_[0], maxRawValues_[1],
                // Center
                maxRawValues_[2], maxRawValues_[3], maxRawValues_[4],
                maxRawValues_[5], maxRawValues_[6], maxRawValues_[7],
                maxRawValues_[8], maxRawValues_[9], maxRawValues_[10],
                maxRawValues_[11], maxRawValues_[12], maxRawValues_[13],
                // Right
                maxRawValues_[14], maxRawValues_[15],

                // MIN
                // Left
                minRawValues_[0], minRawValues_[1],
                // Center
                minRawValues_[2], minRawValues_[3], minRawValues_[4],
                minRawValues_[5], minRawValues_[6], minRawValues_[7],
                minRawValues_[8], minRawValues_[9], minRawValues_[10],
                minRawValues_[11], minRawValues_[12], minRawValues_[13],
                // Right
                minRawValues_[14], minRawValues_[15]);
  // NOLINTEND
}

void IRSensors::readCalibrated() {
  // Emit a warning once, if sensors aren't calibrated
  if(!calibrated_) {
    logger->warning("Sensors not calibrated!");
    calibrated_ = true;
  }

  for(uint8_t i = 0; i < N_SENSORS_; i++) {
    int16_t  value       = 0;
    uint16_t denominator = maxRawValues_[i] - minRawValues_[i];

    if(denominator != 0) {
      value = (((uint16_t)*rawValues[i]) - minRawValues_[i]) *
              MAX_SENSOR_VALUE / denominator;

      value = MAX_SENSOR_VALUE - value;
    }

    if(value < MIN_SENSOR_VALUE)
      value = MIN_SENSOR_VALUE;
    else if(value > MAX_SENSOR_VALUE)
      value = MAX_SENSOR_VALUE;

    sensorValue_[i] = value;
  }
}

void IRSensors::update() {
  /*
   * [!] ATTENTION [!]
   *
   * Time is measured in MILIseconds here
   */
  uint32_t weightedTotal = 0; // this is for the weighted total
  uint16_t total         = 0; // this is for the denominator, which is <= 64000

  isOnLine_ = false;
  readCalibrated();


  // (I) POSITION AND IS ON LINE
  for(uint8_t i = FirstCentral; i <= LastCentral; i++) {
    uint16_t value = sensorValue_[i];

    // Keep track of whether we see the line at all
    if(value > SENSOR_ON_LINE_THRESHOLD) isOnLine_ = true;

    // Only average in values that are above a noise threshold
    if(value > SENSOR_NOISE_THRESHOLD) {
      weightedTotal += value * ((i - FirstCentral) * MAX_SENSOR_VALUE);
      total += value;
    }
  }

  if(!isOnLine_) {
    position_ =
        (previousPosition_ < CENTRAL_POSITION) ? MIN_POSITION : MAX_POSITION;
  } else {
    previousPosition_ = weightedTotal / total;
    position_         = previousPosition_;
  }


  // (II) ERROR
  error_ = position_ - CENTRAL_POSITION;


  // (III) LATERAL MARKS AND IS ON CROSS
  uint32_t currentTime    = Timer::getMiliseconds();
  bool     currentMark[2] = {false};

  currentMark[Left] = sensorValue_[L_1] > SENSOR_ON_LINE_THRESHOLD ||
                      sensorValue_[L_2] > SENSOR_ON_LINE_THRESHOLD;

  currentMark[Right] = sensorValue_[R_1] > SENSOR_ON_LINE_THRESHOLD ||
                       sensorValue_[R_2] > SENSOR_ON_LINE_THRESHOLD;

  // Left mark
  if(currentMark[Left] && !previousMark_[Left] && !currentMark[Right]) {
    // Entering a left mark
    mark_[Left]             = true;
    previousMark_[Left]     = true;
    markDetecionTime_[Left] = currentTime;
  } else if(!currentMark[Left] && previousMark_[Left]) {
    // Leaving a left mark
    mark_[Left]         = false;
    previousMark_[Left] = false;
  }

  // Right mark
  if(!currentMark[Right] && previousMark_[Right]) {
    // Reset mark state; only enters here if a previousMark[Right] was detected
    mark_[Right]         = false;
    previousMark_[Right] = false;

  } else if(currentMark[Right] && !previousMark_[Right] && !currentMark[Left] &&
            (currentTime - markDetecionTime_[Right] > RIGHT_MARK_TIMEOUT)) {
    // When entering a right mark; we register the time of detection
    markDetecionTime_[Right] = currentTime;

  } else if(currentMark[Right] && !previousMark_[Right] && !currentMark[Left] &&
            (currentTime - markDetecionTime_[Right]) <
                RIGHT_MARK_WAITING_LEFT) {
    // Wait the min time to detect a left mark
    // Do nothing...

  } else if(currentMark[Right] && !previousMark_[Right] && !currentMark[Left] &&
            (currentTime - markDetecionTime_[Right]) >
                RIGHT_MARK_WAITING_LEFT) {
    // Entering a right mark
    mark_[Right]         = true;
    previousMark_[Right] = true;
  }

  isOnCross_ = currentMark[Left] && currentMark[Right];
}
