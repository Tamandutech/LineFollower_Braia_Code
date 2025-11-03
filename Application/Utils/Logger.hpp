/*
 * Logger.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef UTILS_LOGGER_HPP_
#define UTILS_LOGGER_HPP_

#include "Timestamp.hpp"
#include <cstdarg>
#include <cstdint>

#define LOGGER_BUFFER_SIZE ((unsigned int) 512)
#define LOGGER_FORMATTED_BUFFER (LOGGER_BUFFER_SIZE + 32)

// MM:SS.mmm (+ space and null terminator)
#define LOGGER_TIMESTAMP_LENGTH 11

class Logger {
public:
  // Defining a type for the log level
  enum Level : uint8_t {
    None    = 0,
    Error   = 1 << 0,
    Warning = 1 << 1,
    Info    = 1 << 2,
    Debug   = 1 << 3
  };

  // Constructor and destructor
  Logger(const char *tag, bool showTag, Level flags);
  ~Logger();

  // Class wide methods
  static void setUseDoubleBreak(bool useDoubleBreak);
  static void setShowLogLevel(bool showLogLevel);
  static void log(const char *format, ...) __attribute__((format(printf, 1, 2)));

  // Per instance methods
  void error(const char *format, ...) __attribute__((format(printf, 2, 3)));
  void warning(const char *format, ...) __attribute__((format(printf, 2, 3)));
  void info(const char *format, ...) __attribute__((format(printf, 2, 3)));
  void debug(const char *format, ...) __attribute__((format(printf, 2, 3)));

private:
  // Class wide variables and methods
  static char buffer[LOGGER_BUFFER_SIZE];
  static char formatted[LOGGER_FORMATTED_BUFFER];
  static bool useDoubleBreak;
  static bool showLogLevel;
  static char timestampStr[LOGGER_TIMESTAMP_LENGTH];
  static bool showTimestamp;
  static Timestamp timestamp;
  
  static void sendLog();
  
  // Per instance variables and methods
  int level;
  bool showTag;
  char *tag;

  void formatLog(const char *logLevelStamp);
};

#endif /* UTILS_LOGGER_HPP_ */

/*
  Maurice Ravel, Bolero
*/