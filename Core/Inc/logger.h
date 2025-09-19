/*
 * logger.h
 *
 *  Created on: Sep 19, 2025
 *      Author: kelvin
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#include <string.h>
#include <stdarg.h>

#define BLE_LOG_BUFFER_SIZE 256
static uint8_t ble_log_buffer[BLE_LOG_BUFFER_SIZE] = {0};

#define bleLog(format, ...) \
	do { \
		snprintf(ble_log_buffer, BLE_LOG_BUFFER_SIZE, \
				 format, ##__VA_ARGS__); \
		ble_log(ble_log_buffer, strlen(ble_log_buffer)); \
	} while (0);

#endif /* INC_LOGGER_H_ */
