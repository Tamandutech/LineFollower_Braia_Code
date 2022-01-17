#ifndef DEBUG_SERVICE_H
#define DEBUG_SERVICE_H

#include "Service.h"

class DebugService : public Service
{
public:
    DebugService(const char* name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~DebugService(){};

    void Setup();
    void Main();

private:
    

};

#endif