#ifndef ESPNOW_SERVICE_H
#define ESPNOW_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "string.h"
#include <cstring>

#include "EspNowHandler.h"
#include "DataManager.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

class ESPNOWService : public Thread
{
public:
    ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;
    uint16_t TaskDelay = 200;
    uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char msgrecebida[60]; // comandos recebidos em texto 24:6f:28:b2:23:d0
    struct CarParameters ParamsData; // Parâmetros do robô
    EspNowHandler *protocolHandler;
    struct PacketData packetReceive;
    struct PacketData packetSend;
    uint8_t *dataReceived; // preparo dos dados recebidos
    bool newData = false; // Informa se uma nova estrutura foi enviada
    uint16_t ptrPos = 0; // informa a posição do ponteiro em que novos dados tem que ser armazenados
    char msgSend[20] = "Mensagem do Robo";
    DataManager *dataManager;
};

#endif