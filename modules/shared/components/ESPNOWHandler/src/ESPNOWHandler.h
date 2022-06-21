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
#include "singleton.hpp"
#include "thread.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

#ifdef ESP32_QEMU
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#else
#include "esp_now.h"
#endif

#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "esp_system.h"

#include "DataAbstract.hpp"

#include "better_console.hpp"

#include "WiFiHandler.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

class ESPNOWHandler : public Thread, public Singleton<ESPNOWHandler>
{
public:
    ESPNOWHandler(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

    uint8_t SendCMD(uint8_t *data, uint16_t size);
    void SendAwnser(uint8_t id, uint8_t *data, uint16_t size);

    bool dataAvailable();                                                      // verifica se existe novo dado para ler
    struct PacketData getPacketReceived(uint8_t uniqueIdCounter, uint8_t num); // obtém o próximo pacote da fila com os pacotes de dados recebidos

private:
    std::string name;

    uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t uniqueIdCounter = 0;

    void ESPNOWInit(uint8_t canal, uint8_t *Mac, bool criptografia); // Inicia o espnow e registra os dados do peer

#ifndef ESP32_QEMU
    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status); // Evento para enviar o dado
#endif
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len); // Evento de dado Recebido

    /// @brief Envia dados via ESPNOW
    /// @param id Identificador do pacote
    /// @param type Tipo de pacote (PacketType)
    /// @param data Dados a serem enviados
    /// @param size Tamanho dos dados a serem enviados
    void Send(uint8_t id, uint16_t type, uint16_t size, uint8_t *data); // envia dados para o gateway

    uint8_t GetUniqueID();

#ifndef ESP32_QEMU
    esp_now_peer_info_t peerInfo; // Variável para adicionar o peer
#endif
    SemaphoreHandle_t xSemaphorePeerInfo;

    PacketData packetReceived;
    static QueueHandle_t queuePacketsReceived;

    static std::list<PacketData> packetsReceived;
    static SemaphoreHandle_t xSemaphorePacketsReceived;
};

#endif