#ifndef DEBUG_SERVICE_H
#define DEBUG_SERVICE_H

#include "Service.h"

class DebugService : public Service
{
public:
    DebugService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Service (name, robot, stackDepth, priority){};
    ~DebugService(){};

    void Setup();
    void Main();

private:
    std::string debugString = "";

};

#endif