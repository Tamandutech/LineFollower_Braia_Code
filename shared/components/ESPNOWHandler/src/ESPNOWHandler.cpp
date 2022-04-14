#include "ESPNOWHandler.h"

std::atomic<ESPNOWHandler *> ESPNOWHandler::instance;
std::mutex ESPNOWHandler::instanceMutex;

std::queue<struct PacketData> ESPNOWHandler::PacketsReceived;
SemaphoreHandle_t ESPNOWHandler::xSemaphorePacketsReceived;

std::queue<struct PacketData> ESPNOWHandler::PacketsToSend;
SemaphoreHandle_t ESPNOWHandler::xSemaphorePacketsToSend;

ESPNOWHandler::ESPNOWHandler(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());
    ESP_LOGD(tag, "Criando Semáforos");

    (xSemaphorePeerInfo) = xSemaphoreCreateMutex();
    (xSemaphorePeerProtocol) = xSemaphoreCreateMutex();
    (xSemaphorePacketsReceived) = xSemaphoreCreateMutex();
    (xSemaphorePacketsToSend) = xSemaphoreCreateMutex();

    this->ESPNOWInit(1, broadcastAddress, false);
}

void ESPNOWHandler::Run()
{
}

void ESPNOWHandler::ESPNOWInit(uint8_t canal, uint8_t *Mac, bool criptografia)
{
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        memcpy(this->peerInfo.peer_addr, Mac, 6);
        this->peerInfo.channel = canal;
        this->peerInfo.encrypt = criptografia;
        this->peerInfo.ifidx = WIFI_IF_STA;
        xSemaphoreGive(xSemaphorePeerInfo);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PeerInfo ocupada, não foi possível definir valor.");
    }
    if (xSemaphoreTake(xSemaphorePeerProtocol, (TickType_t)10) == pdTRUE)
    {
        memcpy(this->peerProtocol.peer_addr, Mac, 6);
        this->peerProtocol.channel = canal;
        this->peerProtocol.encrypt = criptografia;
        this->peerProtocol.ifidx = WIFI_IF_STA;
        xSemaphoreGive(xSemaphorePeerProtocol);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PeerProtocol ocupada, não foi possível definir valor.");
    }
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifndef ESP32_QEMU
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
#endif

#ifndef ESP32_QEMU
    if (esp_now_init() != 0)
        ESP_LOGD("ESP-NOW", "Falha ao iniciar");

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(ESPNOWHandler::OnDataSent);
    // Adiciona peer
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        ESP_LOGD("ESP-NOW", "PeerMac : %x|%x|%x|%x|%x|%x ", this->peerInfo.peer_addr[0], this->peerInfo.peer_addr[1], this->peerInfo.peer_addr[2], this->peerInfo.peer_addr[3], this->peerInfo.peer_addr[4], this->peerInfo.peer_addr[5]);
        if (esp_now_add_peer(&(this->peerInfo)) != ESP_OK)
        {
            ESP_LOGD("ESP-NOW", "Failed to add peer");
            xSemaphoreGive(xSemaphorePeerInfo);
            return;
        }
        else
            xSemaphoreGive(xSemaphorePeerInfo);
    }

    esp_now_register_recv_cb(ESPNOWHandler::OnDataRecv);
#endif
}

esp_err_t ESPNOWHandler::EspSend(uint8_t code, uint16_t ver, uint16_t dataSize, void *msgSend)
{
    esp_err_t sendreturn = ESP_ERR_ESPNOW_NOT_INIT;
    esp_now_peer_info_t peer;

    if (xSemaphoreTake(xSemaphorePeerProtocol, (TickType_t)10) == pdTRUE)
    {
        peer = this->peerProtocol;
        xSemaphoreGive(xSemaphorePeerProtocol);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PeerProtocol ocupada, não foi possível definir valor.");
        return sendreturn;
    }

    struct PacketData Packet;
    Packet.cmd = code;
    Packet.version = ver;
    Packet.packetsToReceive = ceil((float)dataSize / (float)sizeof(Packet.data)) - 1;

    size_t TotalDataSize = sizeof(Packet);
    uint8_t *dataToSend = (uint8_t *)malloc(TotalDataSize);
    uint8_t *msgData = (uint8_t *)msgSend;

    uint16_t packets = Packet.packetsToReceive;
    uint16_t ptrAdvance = 0;                                                              // qtd de bytes que o ponteiro precisa avançar para receber novos dados
    uint16_t lastPacketSize = dataSize - (Packet.packetsToReceive * sizeof(Packet.data)); // Tamanho em bytes do último pacote, os outros pacotes terão um tamanho fixo

    for (int i = packets; i >= 0; i--)
    {
        if (i == 0)
        {
            memcpy(Packet.data, msgData + ptrAdvance, lastPacketSize);
            Packet.packetsize = lastPacketSize;
        }
        else
        {
            memcpy(Packet.data, msgData + ptrAdvance, sizeof(Packet.data));
            Packet.packetsize = sizeof(Packet.data);
        }
        memcpy(dataToSend, &Packet, TotalDataSize);
        sendreturn = esp_now_send(peer.peer_addr, (uint8_t *)dataToSend, TotalDataSize);
        Packet.packetsToReceive--;
        ptrAdvance += sizeof(Packet.data);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    free(dataToSend);
    return sendreturn;
}

void ESPNOWHandler::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    ESP_LOGD("ESP-NOW", "Mensagem recebida, bytes: %d", len);
    struct PacketData packetIncome;
    memcpy(&packetIncome, incomingData, len);
    if (xSemaphoreTake(xSemaphorePacketsReceived, (TickType_t)10) == pdTRUE)
    {
        ESPNOWHandler::PacketsReceived.push(packetIncome);
        xSemaphoreGive(xSemaphorePacketsReceived);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável packetsreceived ocupada, não foi possível definir valor.");
    }
}

void ESPNOWHandler::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGD("OnDataSent", "\r\nLast Packet Send Status:\t");
    ESP_LOGD("OnDataSent", "%s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

bool ESPNOWHandler::dataAvailable()
{
    bool tempvar = false;
    if (xSemaphoreTake(xSemaphorePacketsReceived, (TickType_t)10) == pdTRUE)
    {
        tempvar = !PacketsReceived.empty();
        xSemaphoreGive(xSemaphorePacketsReceived);
        return tempvar;
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável Packetsreceived ocupada, não foi possível definir valor.");
        return tempvar;
    }
}

struct PacketData ESPNOWHandler::getPacketReceived()
{
    struct PacketData tempvar;
    if (xSemaphoreTake(xSemaphorePacketsReceived, (TickType_t)10) == pdTRUE)
    {
        tempvar = ESPNOWHandler::PacketsReceived.front();
        PacketsReceived.pop();
        xSemaphoreGive(xSemaphorePacketsReceived);
        return tempvar;
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PacketsReceived ocupada, não foi possível definir valor.");
        return tempvar;
    }
}