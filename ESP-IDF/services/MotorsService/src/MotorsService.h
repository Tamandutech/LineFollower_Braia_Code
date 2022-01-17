#ifndef MOTORS_SERVICE_H
#define MOTORS_SERVICE_H

#include "Service.h"
#include "ESP32MotorControl.h"

// #include "io.h"

class MotorsService : Service
{
public:
    MotorsService(const char* name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~MotorsService();

    void Main() override;

private:
    

};

#endif