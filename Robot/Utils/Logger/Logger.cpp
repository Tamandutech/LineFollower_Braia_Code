/*
 * Logger.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "../../Utils/Logger/Logger.hpp"

#include "usart.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// TODO this should be owned by BLE class
#define BLE_BUS huart1

// Initialize class variables
char      Logger::buffer[LOGGER_BUFFER_SIZE]            = {0};
char      Logger::formatted[LOGGER_FORMATTED_BUFFER]    = {0};
char      Logger::timestampStr[LOGGER_TIMESTAMP_LENGTH] = {0};
bool      Logger::useDoubleBreak                        = false;
bool      Logger::showLogLevel                          = true;
bool      Logger::showTimestamp                         = true;
Timestamp Logger::timestamp;

Logger::Logger(const char *newTag, bool newShowTag, Level newLevel)
    : tag_(nullptr) {
  level_   = Error | Warning | newLevel;
  showTag_ = newShowTag;

  if(newTag) {
    // Allocate memory for the tag
    // (+3 for the space, colon, and null terminator)
    const int size = strlen(newTag) + 3;
    tag_           = new char[size];

    // Format the incoming string to the instance tag
    snprintf(tag_, size, "%s: ", newTag);
  } else {
    const char *noTag = "";

    tag_ = new char[strlen(noTag)];
    strcpy(tag_, noTag);
  }
}

Logger::~Logger() { delete[] tag_; }

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
           (showTag_) ? tag_ : "", buffer, (useDoubleBreak) ? "\n\n" : "\n");
}

void Logger::sendLog() {
  // TODO treat the return of the function to avoid messing the messages
  HAL_UART_Transmit_DMA(&BLE_BUS, (const uint8_t *)formatted,
                        strlen(formatted));
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

  if((level_ & Info) == 0) return;

  va_start(args, format);
  vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, args);
  formatLog("[I] ");

  sendLog();

  va_end(args);
}

void Logger::debug(const char *format, ...) {
  va_list args;

  if((level_ & Debug) == 0) return;

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

void Logger::logSync(const char *format, ...) {
  char    message[256];
  int     length = 0;
  va_list args;

  va_start(args, format);
  // Reducing two characters to add '\n' and '\0'
  vsnprintf(message, 254, format, args);
  va_end(args);

  length              = strlen(message);
  message[length]     = '\n';
  message[length + 1] = '\0';

  // Size is equal to (lenth + 1) because '\0' is not considered here
  HAL_UART_Transmit(&BLE_BUS, (const uint8_t *)message, length + 1, 50);
}
