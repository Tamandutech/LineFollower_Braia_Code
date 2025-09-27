/*
 * logger.c
 *
 *  Created on: Sep 19, 2025
 *      Author: Kelvin Novais
 */

#include <string.h>

#include "stm32g4xx_hal.h"
#include "logger.h"

#define BLE_LOG_BUFFER_SIZE ((unsigned int) 512)
#define BLE_LOG_FORMATTED_BUFFER (BLE_LOG_BUFFER_SIZE + 256)

extern UART_HandleTypeDef huart1;

const static char *log_start[] =
{
	"",
	"",
	"[E] ",
	"[W] ",
	"[M] ",
	"[I] ",
	"[D] "
};

typedef struct _LoggerData
{
	char buffer[BLE_LOG_BUFFER_SIZE];
	char formatted[BLE_LOG_FORMATTED_BUFFER];
	boolean double_break;
	boolean show_start;
} LoggerData;

static LoggerData ld = {
  .buffer = {0},
  .formatted = {0},
  .double_break = false,
  .show_start = true
};

void _bleLog(LogLevelFlag flag, const char *format, ...) {
	va_list args;

	if (flag == LOG_LEVEL_SILENT
		|| flag >= LOG_LEVEL_N_ELEMENTS)
		return;

	va_start(args, format);

	// FIXME warning
	vsnprintf(ld.buffer, BLE_LOG_BUFFER_SIZE, format, args);

	// FIXME warning
	snprintf(ld.formatted, BLE_LOG_FORMATTED_BUFFER,
			 "%s%s%s",									// format
			 (ld.show_start) ? log_start[flag] : "",	// start (show level)
			 ld.buffer,									// the message
			 (ld.double_break) ? "\n\n" : "\n");		// double break or not

	// TODO treat the return of the function to avoid messing the messages
	HAL_UART_Transmit_DMA(&huart1, (const uint8_t *)ld.formatted, strlen(ld.formatted));

	va_end(args);
}

void bleLogSetDoubleBreak (boolean active) {
	ld.double_break = active;
}
void bleLogSetStartVisible (boolean active) {
	ld.show_start = active;
}
