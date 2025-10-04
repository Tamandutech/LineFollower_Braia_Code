/*
 * logger.h
 *
 *  Created on: Sep 19, 2025
 *      Author: Kelvin Novais
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#include <stdarg.h>

#include "boolean.h"

typedef enum _LogLevelFlag
{
	LOG_LEVEL_SILENT,
	LOG_LEVEL_CUSTOM,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_MESSAGE,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,

	LOG_LEVEL_N_ELEMENTS
} LogLevelFlag;

// The base function for logging
#define USE_CODE_EXPANSION 1

#if USE_CODE_EXPANSION
/*
* A simpler alternative, may be more compatible if we find
* problems with the previous implementation:
*/
#include <stdio.h>
#include <string.h>

#define BLE_LOG_BUFFER_SIZE 1024
extern char ble_log_buffer[BLE_LOG_BUFFER_SIZE];
#define bleLog(format, ...) \
	do { \
		snprintf(ble_log_buffer, BLE_LOG_BUFFER_SIZE, \
			format, ##__VA_ARGS__); \
			ble_log((const uint8_t *) ble_log_buffer, strlen(ble_log_buffer)); \
		} while (0);
		
#else
void _bleLog(LogLevelFlag flag, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

// Variations of logging functions
// TODO add descriptions
#define bleLog(format, ...) _bleLog(LOG_LEVEL_CUSTOM, format, ##__VA_ARGS__)
#define bleLogError(format, ...) _bleLog(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define bleLogWarning(format, ...) _bleLog(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define bleLogMessage(format, ...) _bleLog(LOG_LEVEL_MESSAGE, format, ##__VA_ARGS__)
#define bleLogInfo(format, ...) _bleLog(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define bleLogDebug(format, ...) _bleLog(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define bleLogFor(enabled, format, ...) _bleLog(enabled, format, ##__VA_ARGS__)

void bleLogSetDoubleBreak (boolean active);
void bleLogSetStartVisible (boolean active);
#endif /* USE_CODE_EXPANSION */

#endif /* INC_LOGGER_H_ */
