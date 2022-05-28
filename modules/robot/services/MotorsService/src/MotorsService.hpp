#ifndef MOTORS_SERVICE_H
#define MOTORS_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"

#include "ESP32MotorControl.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class MotorsService : public Thread, public Singleton<MotorsService>
{
public:
    MotorsService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;

    ESP32MotorControl motors;
    CarState state;

    int iloop = 0;
};

#endif