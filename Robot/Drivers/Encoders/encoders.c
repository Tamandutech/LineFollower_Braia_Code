/*
 * encoders.c
 *
 *  Created on: Nov 9, 2025
 *      Author: Kelvin Novais
 *      Author: Samuel Oliveira
 */

#include "../Encoders/encoders.h"

#include "tim.h"

uint32_t           encoderValues[_N_ENCODERS]   = {0};
uint16_t           encoderOverflow[_N_ENCODERS] = {0};
TIM_HandleTypeDef *encoders[_N_ENCODERS]        = {&htim4, &htim3};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /*
   * This function receives a pointer to a timer (the "htim");
   *
   * We run a loop comparing the the received pointer and the pointers stored on
   * "encoders" array, in order to find out which is the index of the array
   */
  enum _Encoder index;
  for(index = 0; index < _N_ENCODERS; index++) {
    if(encoders[index] == htim) break;
  }

  // Checks whether it is overflow (counting up) or underflow (counting down)
  if(__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) {
    // Underflow
    encoderOverflow[index]--;
  } else {
    // Overflow
    encoderOverflow[index]++;
  }

  encoderValues[index] = ((uint32_t)encoderOverflow[index] << 16) +
                         (uint16_t)__HAL_TIM_GET_COUNTER(htim);
}
