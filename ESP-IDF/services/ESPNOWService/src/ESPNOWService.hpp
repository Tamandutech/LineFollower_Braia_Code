#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "string.h"

#include "EspNowHandler.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class ESPNOWService : public Thread
{
public:
    ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    uint16_t TaskDelay = 30;
    uint8_t broadcastAddress[6] = {0x24, 0x6F, 0x28, 0xB2, 0x23, 0xD0};

    EspNowHandler protocolHandler;
    std::string msgSend = "Mensagem do Robo";
};

#endif