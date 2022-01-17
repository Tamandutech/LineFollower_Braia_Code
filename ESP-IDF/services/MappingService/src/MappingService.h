#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include "Service.h"

class MappingService : Service
{
public:
    MappingService(const char* name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~MappingService();

    void Main() override;

private:
    

};

#endif