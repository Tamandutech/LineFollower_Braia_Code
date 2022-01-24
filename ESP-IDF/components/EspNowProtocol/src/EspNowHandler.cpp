#include "EspNowHandler.h"

uint8_t *EspNowHandler::ReceivedData;
bool EspNowHandler::dataReceived;
SemaphoreHandle_t EspNowHandler::xSemaphoreReceivedData;
SemaphoreHandle_t EspNowHandler::xSemaphoredataReceived;

EspNowHandler::EspNowHandler(std::string name)
{
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());
    ESP_LOGD(tag, "Criando Semáforos");
    vSemaphoreCreateBinary(xSemaphorePeerInfo);
    vSemaphoreCreateBinary(xSemaphoreReceivedData);
    vSemaphoreCreateBinary(xSemaphoredataReceived);
}
void EspNowHandler::EspNowInit(uint8_t canal, uint8_t *Mac, bool criptografia)
{
    if (xSemaphoreTake(xSemaphoredataReceived, (TickType_t)10) == pdTRUE)
    {
        EspNowHandler::dataReceived = false;
        xSemaphoreGive(xSemaphoredataReceived);
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável dataReceived ocupada, não foi possível definir valor.");
    }

    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        memcpy(this->peerInfo.peer_addr, Mac, 6);
        this->peerInfo.channel = 1;
        this->peerInfo.encrypt = false;
        xSemaphoreGive(xSemaphorePeerInfo);
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável PeerInfo ocupada, não foi possível definir valor.");
    }
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifiInit();
    espNowInit();
}
esp_err_t EspNowHandler::EspSend(ProtocolCodes code, uint16_t ver, uint16_t dataSize, void *msgSend)
{
    esp_err_t sendreturn = ESP_ERR_ESPNOW_NOT_INIT;
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        int TotalDataSize = sizeof(code) + sizeof(ver) + sizeof(dataSize) + dataSize;
        uint8_t *dataToSend = (uint8_t *)malloc(TotalDataSize);
        uint16_t ptrAdvance = 0; //qtd de bytes que o ponteiro precisa avançar para receber novos dados 
        memcpy(dataToSend, &code, sizeof(code));
        ptrAdvance += sizeof(code);
        memcpy(dataToSend + ptrAdvance, &ver, sizeof(ver));
        ptrAdvance += sizeof(ver);
        memcpy(dataToSend + ptrAdvance, &dataSize, sizeof(dataSize));
        ptrAdvance += sizeof(dataSize);
        memcpy(dataToSend + ptrAdvance, msgSend, dataSize);
        sendreturn = esp_now_send(this->peerInfo.peer_addr, (uint8_t *)dataToSend, TotalDataSize);
        free(dataToSend);
        xSemaphoreGive(xSemaphorePeerInfo);
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável PeerInfo ocupada, não foi possível definir valor.");
    }
    return sendreturn;
}
void EspNowHandler::wifiInit(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}
void EspNowHandler::espNowInit()
{
    if (esp_now_init() != 0)
        ESP_LOGD("ESP-NOW", "Falha ao iniciar");

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(EspNowHandler::OnDataSent);
    // Adiciona peer
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        if (esp_now_add_peer(&this->peerInfo) != ESP_OK)
        {
            ESP_LOGD("ESP-NOW", "Failed to add peer");
            xSemaphoreGive(xSemaphorePeerInfo);
            return;
        }
        else
            xSemaphoreGive(xSemaphorePeerInfo);
    }

    esp_now_register_recv_cb(EspNowHandler::OnDataRecv);
}
void EspNowHandler::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    ESP_LOGD("ESP-NOW", "Mensagem recebida, bytes: %d", len);
    if (xSemaphoreTake(xSemaphoreReceivedData, (TickType_t)10) == pdTRUE)
    {
        memcpy(ReceivedData, incomingData, len);
        ESP_LOGD("ESP-NOW", "Mensagem: %s", ReceivedData);
        xSemaphoreGive(xSemaphoreReceivedData);
        if (xSemaphoreTake(xSemaphoredataReceived, (TickType_t)10) == pdTRUE)
        {
            EspNowHandler::dataReceived = true;
            xSemaphoreGive(xSemaphoredataReceived);
        }
        else
        {
            ESP_LOGE("EspNowHandler", "Variável dataReceived ocupada, não foi possível definir valor.");
        }
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável ReceivedData ocupada, não foi possível definir valor.");
    }
}
void EspNowHandler::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGD("OnDataSent", "\r\nLast Packet Send Status:\t");
    ESP_LOGD("OnDataSent", "%s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
uint8_t *EspNowHandler::getReceivedData()
{
    uint8_t *tempvar = nullptr;
    if (xSemaphoreTake(xSemaphoreReceivedData, (TickType_t)10) == pdTRUE)
    {
        tempvar = EspNowHandler::ReceivedData;
        xSemaphoreGive(xSemaphoreReceivedData);
        if (xSemaphoreTake(xSemaphoredataReceived, (TickType_t)30) == pdTRUE)
        {
            EspNowHandler::dataReceived = false;
            xSemaphoreGive(xSemaphoredataReceived);
            return tempvar;
        }
        else
        {
            ESP_LOGE("EspNowHandler", "Variável dataReceived ocupada, não foi possível definir valor.");
            tempvar = nullptr;
            return tempvar;
        }
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável ReceivedData ocupada, não foi possível definir valor.");
        return tempvar;
    }
}
bool EspNowHandler::dataAvailable()
{
    bool tempvar = false;
    if (xSemaphoreTake(xSemaphoredataReceived, (TickType_t)10) == pdTRUE)
    {
        tempvar = EspNowHandler::dataReceived;
        xSemaphoreGive(xSemaphoredataReceived);
        return tempvar;
    }
    else
    {
        ESP_LOGE("EspNowHandler", "Variável dataReceived ocupada, não foi possível definir valor.");
        return tempvar;
    }
}