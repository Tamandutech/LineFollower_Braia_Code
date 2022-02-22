#ifndef ESPNOW_SERVICE_H
#define ESPNOW_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "string.h"
#include <cstring>

#include "EspNowHandler.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class ESPNOWService : public Thread
{
public:
    ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;
    uint16_t TaskDelay = 200;
    uint8_t broadcastAddress[6] = {0xE0,0xE2,0xE6,0x0D,0x43,0x0C};
    char msgrecebida[60]; // comandos recebidos em texto
    EspNowHandler protocolHandler;
    struct PacketData packetReceive;
    struct PacketData packetSend;
    uint8_t *dataReceived; // preparo dos dados recebidos
    bool newData = false; // Informa se uma nova estrutura foi enviada
    uint16_t ptrPos = 0; // informa a posição do ponteiro em que novos dados tem que ser armazenados
    char msgSend[20] = "Mensagem do Robo";
};

#endif