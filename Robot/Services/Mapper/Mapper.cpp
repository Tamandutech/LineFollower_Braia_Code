/*
 * Mapper.cpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#include "Mapper.hpp"

// Standard headers
#include <cstdint>
#include <cstdio>
#include <vector>

// Headers from our code base
#include "../../Context/GlobalData.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Drivers/Encoders/Encoders.hpp"
#include "../../Drivers/IMU/IMU.hpp"
#include "../../Drivers/IRSensors/IRSensors.hpp"
#include "../../Drivers/Leds/Leds.hpp"
#include "../../Drivers/Motors/Motors.hpp"
#include "../../Drivers/Vacuum/Vacuum.hpp"
#include "../../Services/PID/PID.hpp"
#include "../../Utils/Timer/Timer.hpp"


struct MappingData {
  // FLOATS
  /*
   * 32 bits each float
   */
  float x;
  float y;
  float omega;


  // WORD 1
  /*
   * Can hold up to:       2³⁰ - 1 = 1,073,741,823 µs
   * which is              1,073.7 s
   * which is              17.9 min
   */
  uint32_t timestamp   : 30;
  uint32_t isLeftMark  : 1;
  uint32_t isRightMark : 1;


  // WORD 2
  /*
   * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
   */
  uint32_t leftEncoder : 22;
  /*
   * Must hold up to LastRotatableColor
   */
  uint32_t colorIndex  : 3;
  uint32_t padding1    : 7;


  // WORD 3
  /*
   * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
   */
  uint32_t rightEncoder : 22;
  uint32_t padding2     : 10;

} __attribute__((packed));

enum { TEST = sizeof(MappingData) };
union AlignedPool {
  MappingData as_elems[2000]; /* alignment and typed storage */
  uint8_t     as_bytes[50 * 5000];
};

Logger *Mapper::logger = new Logger("Mapper", false, Logger::Level::Info);

void Mapper::map() {
  /*
   * [!] ATTENTION [!]
   *
   * Time is measured in MICROseconds here
   */
  uint32_t startTime    = 0;
  uint32_t currentTime  = 0;
  uint32_t lastTime     = 0;
  uint32_t outStartTime = 0;
  uint32_t dt           = 0;

  ColorIndex               colorIndex   = FirstRotatableColor;
  MappingData              currentPoint = {0};
  std::vector<MappingData> mapping;
  float                    u = 0;

  bool outWarning      = false;
  bool previousMark[2] = {false};

  globalData.map.clear();
  globalData.map.shrink_to_fit();
  mapping.reserve(TRACK_MAP_N_POINTS);

  logger->info("Mapping...");

  Vacuum::pwmAcceleratedOutput(VACUUM_BASE_PWM);
  IMU::calibrate();

  Encoders::reset();
  IMU::reset();

  startTime = currentTime = lastTime = Timer::getMicroseconds();

  while(globalData.action == Action::Map) {
    currentTime = Timer::getMicroseconds();

    if(currentTime - lastTime >= BASE_LOOP_TIME_US) {
      // UPDATE ROBOT STATE
      dt = currentTime - lastTime;
      IRSensors::update();
      Encoders::update();
      IMU::update(dt);

      // SAVE STATE
      currentPoint.timestamp = currentTime;

      currentPoint.isLeftMark  = false;
      currentPoint.isRightMark = false;

      currentPoint.x     = IMU::position[X];
      currentPoint.y     = IMU::position[Y];
      currentPoint.omega = IMU::angularRate[Yaw];

      currentPoint.leftEncoder  = Encoders::counter[Left];
      currentPoint.rightEncoder = Encoders::counter[Right];

      // CONTROL SIGNAL
      u = PID::evaluate(IRSensors::error);
      Motors::pwmOutputFor(Left, MOTOR_MAPPING_PWM + u);
      Motors::pwmOutputFor(Right, MOTOR_MAPPING_PWM - u);

      // OUT METHOD
      if(IRSensors::isOnLine) {
        // Reset warning
        outWarning = false;
      } else if(!IRSensors::isOnLine && !outWarning) {
        // Register warning, and save time
        outStartTime = currentTime;
        outWarning   = true;
      } else if(outWarning && (lastTime - outStartTime) >= MAX_OUT_TIME_US) {
        // Stop is out of line for MAX_OUT_TIME_US
        logger->info("Stopped: out of line for %lu µs",
                     lastTime - outStartTime); // NOLINT

        globalData.action = Action::None;
        break;
      }

      // RIGHT MARK
      if(IRSensors::mark[Right] && !previousMark[Right]) {
        // Start of the track
        logger->info("Start of the track");
        previousMark[Right] = true;

        Encoders::reset();
        IMU::reset();

        currentPoint.isRightMark = true;
        mapping.push_back(currentPoint);

        Leds::setColorForAll(Green);
        Leds::setColorFor(CenterLed, White);
        Leds::setColorFor(MainBoardLed, White);
      } else if(IRSensors::mark[Right] && previousMark[Right] &&
                (currentTime - startTime) > MIN_TRACK_TIME) {
        // End of the track
        logger->info("End of the track");

        currentPoint.isRightMark = true;
        mapping.push_back(currentPoint);

        Leds::setColorForAll(Black);
        Leds::setColorFor(CenterLed, White);
        Leds::setColorFor(MainBoardLed, White);

        globalData.action = Action::None;
        break;
      }

      // LEFT MARK
      if(IRSensors::mark[Left] && !previousMark[Left]) {
        // Entering a left mark
        previousMark[Left] = true;
        colorIndex         = ColorIndex((colorIndex + 1) % LastRotatableColor);

        currentPoint.isLeftMark = true;
        currentPoint.colorIndex = colorIndex;

        mapping.push_back(currentPoint);

        Leds::setColorForAll(colorIndex);
      } else if(!IRSensors::mark[Left] && previousMark[Left]) {
        // Leaving a left mark
        previousMark[Left] = false;
      }

      // INTERSECTION
      if(IRSensors::isOnCross) {
        Leds::setColorFor(CenterLed, Black);
        Leds::setColorFor(MainBoardLed, Black);
      }

      // REGISTER POINT
      if(Encoders::average >= METERS_TO_PULSES(TRACK_MAP_DISTANCE)) {
        mapping.push_back(currentPoint);
      }

      lastTime = currentTime;
    }
  }

  Motors::stop();
  Vacuum::stopAfter(500);

  // TODO
  // logMap(mapping);
}
