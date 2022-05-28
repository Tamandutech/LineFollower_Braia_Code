#include "ESPNOWHandler.h"

std::list<PacketData> ESPNOWHandler::packetsReceived;
QueueHandle_t ESPNOWHandler::queuePacketsReceived;

ESPNOWHandler::ESPNOWHandler(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->name = name;
    ESP_LOGD(this->name.c_str(), "Criando objeto: %s", name.c_str());
    ESP_LOGD(this->name.c_str(), "Criando Semáforos");

    (xSemaphorePeerInfo) = xSemaphoreCreateMutex();

    queuePacketsReceived = xQueueCreate(10, sizeof(PacketData));

    this->ESPNOWInit(11, broadcastAddress, false);
    this->Start();
}

void ESPNOWHandler::Run()
{
    // Loop
    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queuePacketsReceived, &packetReceived, portMAX_DELAY);

        ESP_LOGD(this->name.c_str(), "Recebido pacote de dados");
        ESP_LOGD(this->name.c_str(), "Tamanho do pacote: %d", packetReceived.size);
        ESP_LOGD(this->name.c_str(), "Dados: %s", packetReceived.data);

        if (packetReceived.type == PACKET_TYPE_CMD)
        {
            std::string ret;
            char *line = (char *)malloc(packetReceived.size);

            memcpy((void *)line, (void *)packetReceived.data, packetReceived.size);

            ESP_LOGD(this->name.c_str(), "Comando recebido via ESPNOW: %s", line);

            esp_err_t err = better_console_run(line, &ret);

            ESP_LOGD(this->name.c_str(), "Retorno do comando:\n%s\nRetorno da execução: %s\n", ret.c_str(), esp_err_to_name(err));

            this->SendAwnser(packetReceived.id, (uint8_t *)ret.c_str(), ret.size() + 1);

            free((void *)line);
        }
        else if (packetReceived.type == PACKET_TYPE_RETURN)
        {
            ESP_LOGD(this->name.c_str(), "Retorno recebido via ESPNOW, ID: %d", packetReceived.id);
            packetsReceived.push_back(packetReceived);
        }
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
        memcpy(this->peerInfo.peer_addr, Mac, canal);
        this->peerInfo.channel = canal;
        this->peerInfo.encrypt = criptografia;
        this->peerInfo.ifidx = WIFI_IF_STA;
        xSemaphoreGive(xSemaphorePeerInfo);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PeerInfo ocupada, não foi possível definir valor.");
    }

#ifndef ESP32_QEMU
    WiFiHandler::getInstance()->wifi_init_sta();

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
    PacketData Packet;
    Packet.id = id;
    Packet.type = type;
    Packet.size = size;
    Packet.numToReceive = size / sizeof(Packet.data) + (size % sizeof(Packet.data) > 0);

    for (size_t i = 0, j = 1; i < size; i += sizeof(Packet.data), j++)
    {
        ESP_LOGD(this->name.c_str(), "Enviando ID: %d | pacote %d de %d", id, j, Packet.numToReceive);

        Packet.numActual = j;
        memcpy(Packet.data, data + i, j == i ? size - i : sizeof(Packet.data));

        esp_now_send(this->peerInfo.peer_addr, (uint8_t *)&Packet, sizeof(PacketData));

        vTaskDelay(0);
    }
}

void ESPNOWHandler::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    ESP_LOGD("ESPNOWHandler", "Mensagem recebida, bytes: %d", len);

    PacketData *Packet = (PacketData *)incomingData;

    xQueueSend(queuePacketsReceived, Packet, 0);
}

void ESPNOWHandler::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGD("ESPNOWHandler", "Status do envio: %s", status == ESP_NOW_SEND_SUCCESS ? "SUCESSO" : "FALHA");
}

PacketData ESPNOWHandler::getPacketReceived(uint8_t uniqueIdCounter, uint8_t num)
{
    // busca pacote com o id e numActual na lista de pacotes recebidos
    auto it = std::find_if(packetsReceived.begin(), packetsReceived.end(), [uniqueIdCounter, num](PacketData &p)
                           { return (p.id == uniqueIdCounter and p.numActual == num); });

    if (it != packetsReceived.end())
    {
        PacketData Packet = *it;
        packetsReceived.erase(it);
        return Packet;
    }
    else
    {
        PacketData Packet;
        Packet.id = 0;
        Packet.type = 0;
        Packet.numActual = 0;
        Packet.numToReceive = 0;
        Packet.size = 0;
        return Packet;
    }
}

uint8_t ESPNOWHandler::GetUniqueID()
{
    this->uniqueIdCounter = this->uniqueIdCounter > 254 ? 0 : this->uniqueIdCounter + 1;

    return this->uniqueIdCounter;
}