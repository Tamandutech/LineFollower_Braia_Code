#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <cstring>
#include <cmath>
#include <queue>

#include "dataEnums.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "esp_system.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class EspNowHandler
{
    public:
        EspNowHandler(std::string name = "EspNowProtocol");
        void EspNowInit(uint8_t canal, uint8_t * Mac, bool criptografia); // Inicia o espnow e registra os dados do peer
        esp_err_t EspSend(ProtocolCodes code, uint16_t ver, uint16_t dataSize, void *msgSend); // envia dados para o gateway
        bool dataAvailable(); // verifica se existe novo dado para ler
        struct PacketData getPacketReceived(); // obtém o próximo pacote da fila com os pacotes de dados recebidos

    private:
        std::string name;
        const char *tag = "EspNowHandler";
        void wifiInit(void);
        void espNowInit();
        static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status); //Evento para enviar o dado
        static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len); // Evento de dado Recebido
        esp_now_peer_info_t peerInfo;
        SemaphoreHandle_t xSemaphorePeerInfo;
        static std::queue<struct PacketData> PacketsReceived;
        static SemaphoreHandle_t xSemaphorepacketreceived;


};

#endif