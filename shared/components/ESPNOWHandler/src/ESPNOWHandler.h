#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <cstring>
#include <cmath>
#include <list>
#include <atomic>
#include <mutex>
#include <algorithm>

#include "dataEnums.h"
#include "thread.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "esp_system.h"

#include "DataAbstract.hpp"
#include "better_console.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

class ESPNOWHandler : public Thread
{
public:
    static ESPNOWHandler *getInstance()
    {
        ESPNOWHandler *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new ESPNOWHandler();
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    void Run() override;

    uint8_t Send(uint16_t type, uint16_t size, uint8_t *data); // envia dados para o gateway
    bool dataAvailable();                                     // verifica se existe novo dado para ler
    struct PacketData getPacketReceived(uint8_t uniqueIdCounter);                    // obtém o próximo pacote da fila com os pacotes de dados recebidos

private:
    std::string name;

    uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    static std::atomic<ESPNOWHandler *> instance;
    static std::mutex instanceMutex;

    uint8_t uniqueIdCounter = 0;

    ESPNOWHandler(std::string name = "ESPNOWHandler", uint32_t stackDepth = 10000, UBaseType_t priority = 9);

    void ESPNOWInit(uint8_t canal, uint8_t *Mac, bool criptografia); // Inicia o espnow e registra os dados do peer

    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);    // Evento para enviar o dado
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len); // Evento de dado Recebido

    uint8_t GetUniqueID();

    esp_now_peer_info_t peerInfo; // Variável para adicionar o peer
    SemaphoreHandle_t xSemaphorePeerInfo;

    esp_now_peer_info_t peerProtocol; // Variável para ler dados do peeer nos métodos da classe
    SemaphoreHandle_t xSemaphorePeerProtocol;

    static std::list<PacketData> PacketsReceived;
    static SemaphoreHandle_t xSemaphorePacketsReceived;
};

#endif