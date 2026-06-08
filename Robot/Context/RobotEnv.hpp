/*
 * RobotEnv.hpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_ROBOTENV_HPP_
#define CONTEXT_ROBOTENV_HPP_

#include <cmath>
#include <cstdint>

/*
 * Note: If using C++17 or later, prefer using "inline constexpr" for constants,
 * in order to avoid it being copied on every file this header is included.
 */
namespace RobotEnv {
const int32_t ROBOT_WIDTH = 4;


const uint16_t PULSES_PER_REVOLUTION = 4095; // pulses/rev


const float WHEEL_DIAMETER      = 0.0222F;               // m
const float WHEEL_RADIUS        = WHEEL_DIAMETER / 2.0F; // m
const float WHEEL_CIRCUMFERENCE = M_PI * WHEEL_DIAMETER; // m


const float MM_PER_PULSE =
    WHEEL_CIRCUMFERENCE / (PULSES_PER_REVOLUTION * 1000); // mm


const uint8_t  TRACK_MAX_LENGTH = 60;                               // m
const uint32_t TRACK_MAX_PULSES =
    TRACK_MAX_LENGTH * PULSES_PER_REVOLUTION / WHEEL_CIRCUMFERENCE; // pulses
const float  TRACK_MAP_DISTANCE = 0.02F;                            // m
const size_t TRACK_MAP_N_POINTS = TRACK_MAX_LENGTH / TRACK_MAP_DISTANCE;


const float GRAVITY       = 9.80665;           // m/s²
const float MILLI_GRAVITY = GRAVITY / 1000.0F; // m/s²


const float kp = 0.1;
const float kd = 1.9;


const uint16_t BASE_LOOP_TIME_US = 1000;  // µs
const uint16_t MAX_OUT_TIME_US   = 60000; // µs
// The min time to consider valid a second reading of the right mark
const uint32_t MIN_TRACK_TIME = 2 * 1e6; // µs


const int32_t MOTOR_BASE_PWM           = 85;    // ‰
const float   MOTOR_BASE_SPEED         = 0.75F; // m/s
const int16_t MOTOR_MAX_PWM            = 1000;  // ‰
const float   MOTOR_MAX_SPEED          = 4;     // m/s
const int32_t MOTOR_MAX_DECELERATION   = 4;     // m/s
const int32_t MOTOR_MAX_ROTATION_SPEED = 4;     // m/s
const uint8_t MOTOR_BRAKE_TIME         = 75;    // ms
const int32_t MOTOR_MAPPING_PWM        = 85;    // ‰


const uint16_t VACUUM_BASE_PWM                    = 150;  // ‰
const uint16_t VACUUM_MIN_PWM                     = 100;  // ‰
const uint16_t VACUUM_MAX_PWM                     = 1000; // ‰
const uint8_t  VACUUM_INTERVAL_BETWEEN_INCREMENTS = 2;    // ms
const uint16_t VACUUM_MAPPING_PWM                 = 130;  // ‰


} // namespace RobotEnv

#define METERS_TO_PULSES(m) \
  (int32_t)((m * RobotEnv::PULSES_PER_REVOLUTION) / \
            RobotEnv::WHEEL_CIRCUMFERENCE)

#endif /* CONTEXT_ROBOTENV_HPP_ */

// Bach - Cello Suite no. 1 in G major BWV 1007
