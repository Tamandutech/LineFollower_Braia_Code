#include "ESPNOWHandler.h"

std::atomic<ESPNOWHandler *> ESPNOWHandler::instance;
std::mutex ESPNOWHandler::instanceMutex;

std::list<PacketData> ESPNOWHandler::PacketsReceived;
SemaphoreHandle_t ESPNOWHandler::xSemaphorePacketsReceived;

ESPNOWHandler::ESPNOWHandler(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->name = name;
    ESP_LOGD(this->name.c_str(), "Criando objeto: %s", name.c_str());
    ESP_LOGD(this->name.c_str(), "Criando Semáforos");

    (xSemaphorePeerInfo) = xSemaphoreCreateMutex();
    (xSemaphorePeerProtocol) = xSemaphoreCreateMutex();
    (xSemaphorePacketsReceived) = xSemaphoreCreateMutex();

    this->ESPNOWInit(1, broadcastAddress, false);
}

void ESPNOWHandler::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Loop
    for (;;)
    {
        if (PacketsReceived.size() > 0)
        {
            xSemaphoreTake(xSemaphorePacketsReceived, portMAX_DELAY);

            for (auto packet = PacketsReceived.begin(); packet != PacketsReceived.end(); ++packet)
            {
                ESP_LOGD(this->name.c_str(), "Recebido pacote de dados");
                ESP_LOGD(this->name.c_str(), "Tamanho do pacote: %d", packet->size);
                ESP_LOGD(this->name.c_str(), "Dados: %s", packet->data);

                switch (packet->type)
                {
                case PACKET_TYPE_CMD:
                    std::string ret;
                    const char *line = (const char *)malloc(packet->size);

                    memcpy((void *)line, (void *)packet->data, packet->size);

                    ESP_LOGD(this->name.c_str(), "Comando recebido via ESPNOW: %s", line);

                    esp_err_t err = better_console_run(line, &ret);

                    ESP_LOGD(this->name.c_str(), "Retorno do comando: %s, retorno da execução: %s", ret.c_str(), esp_err_to_name(err));

                    this->SendAwnser(packet->id, (uint8_t *)ret.c_str(), ret.size() + 1);

                    free((void *)line);

                    PacketsReceived.erase(packet);

                    break;
                }
            }

            xSemaphoreGive(xSemaphorePacketsReceived);
        }
        else
            this->Suspend();

        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}

/// @brief Inicia o ESPNOW e registra os dados do peer
/// @param canal Canal do ESPNOW
/// @param Mac Endereço MAC do peer
/// @param criptografia Ativa ou desativa a criptografia
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

uint8_t ESPNOWHandler::SendCMD(uint8_t *data, uint16_t size)
{
    uint8_t id = GetUniqueID();
    this->Send(id, PACKET_TYPE_CMD, size, data);
    return id;
}

void ESPNOWHandler::SendAwnser(uint8_t id, uint8_t *data, uint16_t size)
{
    this->Send(id, PACKET_TYPE_RETURN, size, data);
}

void ESPNOWHandler::Send(uint8_t id, uint16_t type, uint16_t size, uint8_t *data)
{
    esp_now_peer_info_t peer;
    peer = this->peerProtocol;

    PacketData Packet;
    Packet.id = id;
    Packet.type = type;
    Packet.size = size;
    memcpy(Packet.data, data, size);

    ESP_LOGD(this->name.c_str(), "Enviando pacote de ID: %d, tipo: %d, tamanho: %d, mensagem: %s", Packet.id, Packet.type, Packet.size, (const char *)Packet.data);

    esp_now_send(peer.peer_addr, (uint8_t *)&Packet, sizeof(PacketData));
}

void ESPNOWHandler::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    ESP_LOGD("ESPNOWHandler", "Mensagem recebida, bytes: %d", len);

    if (xSemaphoreTake(xSemaphorePacketsReceived, (TickType_t)10) == pdTRUE)
    {
        PacketData packet;

        packet.id = ((PacketData *)incomingData)->id;
        packet.type = ((PacketData *)incomingData)->type;
        packet.size = ((PacketData *)incomingData)->size;
        memcpy(packet.data, ((PacketData *)incomingData)->data, packet.size);

        ESP_LOGD("ESPNOWHandler", "Recebido pacote de ID: %d, tipo: %d, tamanho: %d, mensagem: %s", packet.id, packet.type, packet.size, (const char *)packet.data);

        ESPNOWHandler::PacketsReceived.push_back(packet);
        xSemaphoreGive(xSemaphorePacketsReceived);

        ESPNOWHandler::getInstance()->Resume();
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

PacketData ESPNOWHandler::getPacketReceived(uint8_t uniqueIdCounter)
{
    // busca pacote com o id na lista de pacotes recebidos
    auto it = std::find_if(PacketsReceived.begin(), PacketsReceived.end(), [uniqueIdCounter](PacketData &p)
                           { return p.id == uniqueIdCounter; });

    if (it != PacketsReceived.end())
    {
        PacketData Packet = *it;
        PacketsReceived.erase(it);
        return Packet;
    }
    else
    {
        PacketData Packet;
        Packet.id = 0;
        Packet.type = 0;
        Packet.size = 0;
        return Packet;
    }
}

uint8_t ESPNOWHandler::GetUniqueID()
{
    this->uniqueIdCounter = this->uniqueIdCounter > 254 ? 0 : this->uniqueIdCounter + 1;

    return this->uniqueIdCounter;
}