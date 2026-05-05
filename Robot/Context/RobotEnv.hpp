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
const int32_t  ROBOT_WIDTH           = 4;
const double   WHEEL_DIAMETER        = 0.0222;                // m
const uint16_t PULSES_PER_REVOLUTION = 1024;                  // pulses/rev
const double   WHEEL_RADIUS          = WHEEL_DIAMETER / 2.0;  // m
const double   WHEEL_CIRCUMFERENCE   = M_PI * WHEEL_DIAMETER; // m
const double   MM_PER_PULSE = WHEEL_CIRCUMFERENCE / PULSES_PER_REVOLUTION; // mm
const float    GRAVITY      = 9.80665;            // m/s²
const float    MILLI_GRAVITY = GRAVITY / 1000.0F; // m/s²

// Tolerance for floating point comparisons
constexpr float EPSILON_TOLERANCE = 1e-6F;

// Constants for preventing integral windup in PID
const float INTEGRAL_MAX = 1000.0F;  // Maximum value for the full term
const float INTEGRAL_MIN = -1000.0F; // Minimum value for the full term

const float kp = 0.1;
const float kd = 1.9;

// struct PIDParameter {
//   float base;
//   float translational;
// };

// const PIDParameter Kp = {.base = 0.100, .translational = 01.000}; // NOLINT
// const PIDParameter Kd = {.base = 1.900, .translational = 20.000}; // NOLINT
// const PIDParameter Ki = {.base = 0.000, .translational = 04.600}; // NOLINT

const uint16_t BASE_LOOP_TIME_US = 1000;                  // µs

const int32_t MOTOR_BASE_PWM           = 100;             // ‰
const float   MOTOR_BASE_SPEED         = 0.75F;           // m/s
const int16_t MOTOR_MAX_PWM            = 1000;            // ‰
const float   MOTOR_MAX_SPEED          = 4;               // m/s
const int32_t MOTOR_MAX_DECELERATION   = 4;               // m/s
const int32_t MOTOR_MAX_ROTATION_SPEED = 4;               // m/s

const uint16_t VACUUM_BASE_PWM                    = 150;  // ‰
const uint16_t VACUUM_MIN_PWM                     = 100;  // ‰
const uint16_t VACUUM_MAX_PWM                     = 1000; // ‰
const uint8_t  VACUUM_INTERVAL_BETWEEN_INCREMENTS = 2;    // ms

} // namespace RobotEnv

#define METERS_TO_PULSES(m) \
  (int32_t)((m * RobotEnv::PULSES_PER_REVOLUTION) / \
            RobotEnv::WHEEL_CIRCUMFERENCE)

#endif /* CONTEXT_ROBOTENV_HPP_ */
