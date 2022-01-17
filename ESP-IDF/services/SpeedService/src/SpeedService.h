#ifndef SPEED_SERVICE_H
#define SPEED_SERVICE_H

#include "Service.h"

class SpeedService : Service
{
public:
    SpeedService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~SpeedService();

    void Main() override;

private:
    

};

#endif