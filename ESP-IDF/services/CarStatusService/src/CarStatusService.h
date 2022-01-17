#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "Service.h"

class CarStatusService : Service
{
public:
    CarStatusService(const char* name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~CarStatusService();

    void Main() override;

private:
    

};

#endif