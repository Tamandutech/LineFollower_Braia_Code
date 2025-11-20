/*
 * RobotEnv.hpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_ROBOTENV_HPP_
#define CONTEXT_ROBOTENV_HPP_

#include <cstdint>

// TODO set all pins here

namespace RobotEnv {
// TODO set unit (prefer SI [m])
const int32_t ROBOT_WIDTH         = 4;
const int32_t WHEEL_RADIUS        = 11;
const int32_t WHEEL_CIRCUMFERENCE = 70;

// const int32_t BASE_MOTOR_PWM  = 10;

const int16_t MAX_MOTOR_PWM = 1000;

// Tolerance for floating point comparisons
constexpr float EPSILON_TOLERANCE = 1e-6F;

// Constants for preventing integral windup in PID
constexpr float INTEGRAL_MAX = 1000.0F;  // Maximum value for the full term
constexpr float INTEGRAL_MIN = -1000.0F; // Minimum value for the full term

// TODO remove namespace
namespace PID {
const float kp = 0.1;
const float kd = 1.9;
} // namespace PID

// TODO remove namespace
namespace MotorDriver {
// TODO set unit (prefer SI [m/s])
const float   MAX_SPEED          = 1000; // TODO this is a PWM value by now
const int32_t MAX_DECELERATION   = 4;
const int32_t MAX_ROTATION_SPEED = 4;
} // namespace MotorDriver

// TODO remove namespace
namespace VacuumDriver {
const uint16_t BASE_VACUUM_PWM             = 170;
const uint8_t  INTERVAL_BETWEEN_INCREMENTS = 2; // ms
} // namespace VacuumDriver

} // namespace RobotEnv

#endif /* CONTEXT_ROBOTENV_HPP_ */
