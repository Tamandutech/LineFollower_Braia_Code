/*
 * main.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "application.h"

#include "Utils/Logger.hpp"
#include "Utils/Timer.hpp"

#include "Drivers/LedDriver/LedDriver.hpp"
#include "Drivers/MotorDriver/MotorDriver.hpp"
#include "Drivers/EncoderDriver/EncoderDriver.hpp"
#include "Drivers/VacuumDriver/VacuumDriver.hpp"
#include "Drivers/QTRSensorDriver/QTRSensorDriver.hpp"

// TODO tmp
#include "platform_functions.h"

/* 
 * Here we declare private (aka static) variables to this file, but they are
 * still sharede between functions
 */
static Logger *logger = new Logger("Main", true,
static_cast<Logger::Level>(Logger::Level::Debug | Logger::Level::Info));

void setup(void) {
  logger->info("Robot is starting...");
  Timer::delayMiliseconds(50);

  // TODO tmp
  mcu_start();
  // TODO tmp
  // imu_init(&imu_ctx, &int1_route);

  QTRSensorDriver::calibrateSensors();

  LedDriver::setColorForAll(LedDriver::Colors.blue);
  Timer::delayMiliseconds(500);

  LedDriver::setColorFor(LedDriver::Colors.magenta, LedDriver::Leds::Center);
  Timer::delayMiliseconds(500);
  
}

void loop(void) {
  // ...
  Timer::delayMiliseconds(1000);
  
  logger->debug("Encoders L:%05lu R:%05lu",
    EncoderDriver::getCounter(EncoderDriver::Encoder::Left), 
    EncoderDriver::getCounter(EncoderDriver::Encoder::Right));
}
