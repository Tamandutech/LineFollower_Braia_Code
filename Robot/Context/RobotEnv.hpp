/*
 * RobotEnv.hpp
 *
 *  Created on: Nov 8, 2025
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_ROBOTENV_HPP_
#define CONTEXT_ROBOTENV_HPP_

#include <cmath>


#define ROBOT_WIDTH 4


#define PULSES_PER_REVOLUTION 4095 // pulses/rev


#define WHEEL_DIAMETER      0.0222F                 // m
#define WHEEL_RADIUS        (WHEEL_DIAMETER / 2.0F) // m
#define WHEEL_CIRCUMFERENCE (M_PI * WHEEL_DIAMETER) // m


#define MM_PER_PULSE \
  (WHEEL_CIRCUMFERENCE / (PULSES_PER_REVOLUTION * 1000)) // mm


#define TRACK_MAX_LENGTH 60                                        // m
#define TRACK_MAX_PULSES \
  (TRACK_MAX_LENGTH * PULSES_PER_REVOLUTION / WHEEL_CIRCUMFERENCE) // pulses
#define TRACK_MAP_DISTANCE 0.02F                                   // m
#define TRACK_MAP_N_POINTS (TRACK_MAX_LENGTH / TRACK_MAP_DISTANCE)


#define GRAVITY       9.80665F            // m/s²
#define MILLI_GRAVITY (GRAVITY / 1000.0F) // m/s²


#define kp 0.207F
#define kd 2.05F


#define BASE_LOOP_TIME_US 1000  // µs
#define MAX_OUT_TIME_US   60000 // µs
// The min time to consider valid a second reading of the right mark
#define MIN_TRACK_TIME    (2 * 1e6) // µs


#define MOTOR_BASE_PWM           220   // ‰
#define MOTOR_BASE_SPEED         0.75F // m/s
#define MOTOR_MAX_PWM            1000  // ‰
#define MOTOR_MAX_SPEED          4     // m/s
#define MOTOR_MAX_DECELERATION   4     // m/s
#define MOTOR_MAX_ROTATION_SPEED 4     // m/s
#define MOTOR_BRAKE_TIME         75    // ms
#define MOTOR_MAPPING_PWM        220   // ‰


#define VACUUM_BASE_PWM                    900  // ‰
#define VACUUM_MIN_PWM                     100  // ‰
#define VACUUM_MAX_PWM                     1000 // ‰
#define VACUUM_INTERVAL_BETWEEN_INCREMENTS 2    // ms
#define VACUUM_MAPPING_PWM                 1000 // ‰


#define METERS_TO_PULSES(m) \
  (int32_t)((m * PULSES_PER_REVOLUTION) / WHEEL_CIRCUMFERENCE)


#endif /* CONTEXT_ROBOTENV_HPP_ */

// Bach - Cello Suite no. 1 in G major BWV 1007
