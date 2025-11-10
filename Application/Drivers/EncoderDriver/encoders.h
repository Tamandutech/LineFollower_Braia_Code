/*
 * encoders.h
 *
 *  Created on: Nov 9, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERDRIVER_ENCODERS_H_
#define DRIVERS_ENCODERDRIVER_ENCODERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g4xx_hal.h"
#include <stdint.h>

enum _Encoder {
  Left = 0,
  Right,

  _N_ENCODERS
};

extern uint32_t           encoderValues[_N_ENCODERS];
extern uint16_t           encoderOverflow[_N_ENCODERS];
extern TIM_HandleTypeDef *encoders[_N_ENCODERS];

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_ENCODERDRIVER_ENCODERS_H_ */
