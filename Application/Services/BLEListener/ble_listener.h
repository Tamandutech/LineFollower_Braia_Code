/*
 * ble_listening.h
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_BLELISTENER_BLE_LISTENER_H_
#define SERVICES_BLELISTENER_BLE_LISTENER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void start_ble_listening(void);

extern volatile uint8_t run;

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_BLELISTENER_BLE_LISTENER_H_ */
