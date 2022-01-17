#ifndef DEBUG_SERVICE_H
#define DEBUG_SERVICE_H

#include "Service.h"

class DebugService : public Service
{
public:
    DebugService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~Service() override;

    void Setup() override;
    void Main() override;

private:
    

};

#endif