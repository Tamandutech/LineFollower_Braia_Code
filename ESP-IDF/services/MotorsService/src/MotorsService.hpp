#ifndef MOTORS_SERVICE_H
#define MOTORS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "ESP32MotorControl.h"

#include "io.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class MotorsService : public Thread
{
public:
    MotorsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;

    ESP32MotorControl motors;
};

#endif