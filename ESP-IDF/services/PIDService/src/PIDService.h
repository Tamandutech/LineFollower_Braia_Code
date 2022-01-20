#ifndef PID_SERVICE_H
#define PID_SERVICE_H

#include "Service.h"

class PIDService : Service
{
public:
    PIDService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Service (name, robot, stackDepth, priority){};
    ~PIDService();

    void Main() override;

private:
    

};

#endif