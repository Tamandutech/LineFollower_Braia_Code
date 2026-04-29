/*
 * Logger.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "../../../Robot/Utils/Logger/Logger.hpp"

#include "stm32g4xx_hal.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// Initialize class variables
char      Logger::buffer[LOGGER_BUFFER_SIZE]            = {0};
char      Logger::formatted[LOGGER_FORMATTED_BUFFER]    = {0};
char      Logger::timestampStr[LOGGER_TIMESTAMP_LENGTH] = {0};
bool      Logger::useDoubleBreak                        = false;
bool      Logger::showLogLevel                          = true;
bool      Logger::showTimestamp                         = true;
Timestamp Logger::timestamp;

extern "C" {
extern UART_HandleTypeDef huart1;
}

Logger::Logger(const char *newTag, bool newShowTag, Level newLevel)
    : tag(nullptr) {
  level   = Error | Warning | newLevel;
  showTag = newShowTag;

  if(newTag) {
    // Allocate memory for the tag
    // (+3 for the space, colon, and null terminator)
    const int size = strlen(newTag) + 3;
    tag            = new char[size];

    // Format the incoming string to the instance tag
    snprintf(tag, size, "%s: ", newTag);
  } else {
    const char *noTag = "";

    tag = new char[strlen(noTag)];
    strcpy(tag, noTag);
  }
}

Logger::~Logger() { delete[] tag; }

void Logger::formatLog(const char *logLevelStamp) {
  // MM:SS.mmm
  uint8_t  minutes = 0, seconds = 0;
  uint32_t milliseconds = 0;

  timestamp.getTimestamp(&minutes, &seconds, &milliseconds);
  snprintf(timestampStr, LOGGER_TIMESTAMP_LENGTH, "%02d:%02d.%03lu ", minutes,
           seconds, milliseconds);

  // Timestamp [logLevelStamp] tag: buffer useDoubleBreak
  snprintf(formatted, LOGGER_FORMATTED_BUFFER, "%s%s%s%s%s",
           (showTimestamp) ? timestampStr : "",
           (showLogLevel && logLevelStamp) ? logLevelStamp : "",
           (showTag) ? tag : "", buffer, (useDoubleBreak) ? "\n\n" : "\n");
}

void Logger::sendLog() {
  // TODO treat the return of the function to avoid messing the messages
  HAL_UART_Transmit_DMA(&huart1, (const uint8_t *)formatted, strlen(formatted));
}

void Logger::error(const char *format, ...) {
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);
  formatLog("[E] ");

  sendLog();

  va_end(args);
}

void Logger::warning(const char *format, ...) {
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);
  formatLog("[W] ");

  sendLog();

  va_end(args);
}

void Logger::info(const char *format, ...) {
  va_list args;

  if((level & Info) == 0) return;

  va_start(args, format);
  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);
  formatLog("[I] ");

  sendLog();

  va_end(args);
}

void Logger::debug(const char *format, ...) {
  va_list args;

  if((level & Debug) == 0) return;

  va_start(args, format);
  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);
  formatLog("[D] ");

  sendLog();

  va_end(args);
}

void Logger::setUseDoubleBreak(bool newUseDoubleBreak) {
  useDoubleBreak = newUseDoubleBreak;
}

void Logger::setShowLogLevel(bool newShowLogLevel) {
  showLogLevel = newShowLogLevel;
}

void Logger::setShowTimestamp(bool newShowTimestamp) {
  showTimestamp = newShowTimestamp;
}

void Logger::resetTimestamp() { timestamp.reset(); }

void Logger::log(const char *format, ...) {
  va_list args;

  va_start(args, format);

  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);

  snprintf(formatted, LOGGER_FORMATTED_BUFFER,
           "%s%s",                            // format
           buffer,                            // the message
           (useDoubleBreak) ? "\n\n" : "\n"); // double break or not

  sendLog();

  va_end(args);
}
