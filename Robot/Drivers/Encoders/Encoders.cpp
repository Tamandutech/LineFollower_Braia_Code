/*
 * Encoders.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

/******************************************************************************/
// INCLUDES
#include "Encoders.hpp"

#include "../../Utils/Logger/Logger.hpp"

#include "tim.h"

/******************************************************************************/
// PERIPHERALS
struct Pin {
  TIM_HandleTypeDef *timerHandle;
  uint32_t           channel;
};

static Pin pins[N_SIDES_] = {
  [Left] = {&htim4, TIM_CHANNEL_ALL},
  [Right] = {&htim3, TIM_CHANNEL_ALL}
};


/******************************************************************************/
// VARIABLES
// (I) C compatible private variables
/*
 * Here we declare static variables to make them "private" to this file, but
 * still visible to the C callback function
 */
static uint32_t encoderValue[N_SIDES_]    = {0};
static uint16_t encoderOverflow[N_SIDES_] = {0};

// (II) C++ Private
static Logger *logger = new Logger("Encoders", true, Logger::Level::All);
int32_t        Encoders::average_           = 0;
int32_t        Encoders::counter_[N_SIDES_] = {0};

// (III) C++ Public
const int32_t &Encoders::average             = average_;
const int32_t (&Encoders::counter)[N_SIDES_] = counter_;

void Encoders::initialize() {
  static bool initialized = false;

  if(initialized) {
    logger->error("Encoders already initialized, unexpected behaviour");
  }

  HAL_TIM_Encoder_Start(pins[Left].timerHandle, pins[Left].channel);
  HAL_TIM_Encoder_Start(pins[Right].timerHandle, pins[Right].channel);

  // Enables overflow/underflow interrupt
  __HAL_TIM_ENABLE_IT(pins[Left].timerHandle, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(pins[Right].timerHandle, TIM_IT_UPDATE);

  initialized = true;
}

void Encoders::update() {
  average_ = 0;

  // Get individual encoder values
  for(uint8_t i = 0; i < N_SIDES_; i++) {
    encoderValue[i] = ((uint32_t)encoderOverflow[i] << 16) +
                      (uint16_t)__HAL_TIM_GET_COUNTER(pins[i].timerHandle);

    counter_[i] = encoderValue[i];
    average_ += encoderValue[i];
  }

  // Compute average
  average_ /= N_SIDES_;
}

void Encoders::setCounter(Side index, uint32_t value) {
  if(index >= N_SIDES_) {
    logger->error("Invalid encoder");
    index = Left;
  }

  encoderValue[index]    = value;
  encoderOverflow[index] = value >> 16;

  __HAL_TIM_SET_COUNTER(pins[index].timerHandle, (uint16_t)(value & 0xFFFF));

  // Update to reflect the changes on exposed variables
  update();
}

void Encoders::reset() {
  for(uint8_t index = 0; index < N_SIDES_; index++) {
    __HAL_TIM_SET_COUNTER(pins[index].timerHandle, 0);
    encoderValue[index]    = 0;
    encoderOverflow[index] = 0;
  }

  // Update to reflect the changes on exposed variables
  update();
}

// The callback must be at C scope
extern "C" {
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /*
   * This function receives a pointer to a timer (the "htim");
   *
   * We run a loop comparing the the received pointer and the pointers stored on
   * "encoders" array, in order to find out which is the index of the array
   */
  uint8_t index = 0;
  for(index = 0; index < N_SIDES_; index++) {
    if(pins[index].timerHandle == htim) break;
  }

  // Checks whether it is overflow (counting up) or underflow (counting down)
  if(__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) {
    // Underflow
    encoderOverflow[index]--;
  } else {
    // Overflow
    encoderOverflow[index]++;
  }

  encoderValue[index] = ((uint32_t)encoderOverflow[index] << 16) +
                        (uint16_t)__HAL_TIM_GET_COUNTER(htim);
}
}
