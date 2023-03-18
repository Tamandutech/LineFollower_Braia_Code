#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED)

#include "BLEServerService.hpp"

QueueHandle_t BLEServerService::queuePacketsReceived;


/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client conectado.");
        BLEServerService::getInstance()->deviceConnected = true;
    };
    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Endereço do client: %s", NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 3x interval time for best results.
         */
        BLEServerService::getInstance()->deviceConnected = true;
    };
    void onDisconnect(NimBLEServer *pServer)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client disconnected - start advertising");
        BLEServerService::getInstance()->deviceConnected = false;
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "MTU updated: %u for connection ID: %u", MTU, desc->conn_handle);
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Server Passkey Request");
        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "The passkey YES/NO number: %d", pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            /** NOTE: createServer returns the current server reference unless one is not already created */
            NimBLEDevice::createServer()->disconnect(desc->conn_handle);
            ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Encrypt connection failed - disconnecting client");
            return;
        }
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s : onRead(), value: %s",
                 pCharacteristic->getUUID().toString().c_str(),
                 pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc)
    {
        ble_gatt_uart_packet_t tempPacket;

        tempPacket.len = pCharacteristic->getValue().length();
        tempPacket.payload = (uint8_t *)malloc(tempPacket.len + 1);
        tempPacket.clientMAC = desc->peer_id_addr;

        strcpy((char *)tempPacket.payload, pCharacteristic->getValue().c_str());

        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s : onWrite(), value: %s",
                 pCharacteristic->getUUID().toString().c_str(),
                 pCharacteristic->getValue().c_str());

        if (pCharacteristic->getUUID() == NimBLEUUID(CHARACTERISTIC_UUID_RX))
        {
            xQueueSend(BLEServerService::queuePacketsReceived, &tempPacket, 0);
        }
    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Sending notification to clients");
    };

    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Notification/Indication status code: %d , return code: %d, %s",
                 status,
                 code,
                 NimBLEUtils::returnCodeToString(code));
    };
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks
{
    void onWrite(NimBLEDescriptor *pDescriptor)
    {
        std::string dscVal((char *)pDescriptor->getValue(), pDescriptor->getLength());
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Descriptor witten value: %s", dscVal.c_str());
    };

    void onRead(NimBLEDescriptor *pDescriptor)
    {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "%s Descriptor read", pDescriptor->getUUID().toString().c_str());
    };
    ;
};

/** Define callback instances globally to use for multiple Charateristics \ Descriptors */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

// Função executada pela task de stream
void RunStream(void *pvParameters)
{
    auto bleservice = BLEServerService::getInstance();
    auto datamanager = DataManager::getInstance();
    auto status = Robot::getInstance()->getStatus();
    const char *Tag = pcTaskGetName(bleservice->xTaskStream);
    //esp_log_level_set(Tag,ESP_LOG_DEBUG);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const int TaskDelay = 20;

    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
        //ESP_LOGD(Tag,"Bluetooth Conectado: %d, Itens para stream: %d", bleservice->deviceConnected, datamanager->NumItemsReadyStream());
        if(datamanager->NumItemsReadyStream() > 0 && bleservice->deviceConnected && (CarState)status->robotState->getData() != CAR_STOPPED)
        {
            cJSON* StreamData = datamanager->getStreamData();
            char *json_Data = cJSON_PrintUnformatted(StreamData);

            ESP_LOGD(Tag, "Enviando pacote de dados, tamanho: %d", strlen(json_Data));
            // Enviar pacote de dados para o cliente via BLE GATT chunks do tamanho de 200 bytes
            size_t msgSize = strlen(json_Data) + 1; // +1 no final para pegar /0
            for (size_t i = 0; msgSize > 0; i += 200, msgSize -= bleservice->pStreamTxCharacteristic->getValue().size())
            {
                bleservice->pStreamTxCharacteristic->setValue((uint8_t *)&json_Data[i], msgSize < 200 ? msgSize : 200);
                bleservice->pStreamTxCharacteristic->notify();
                ESP_LOGD(Tag, "Pacote enviado, tamanho: %d", bleservice->pStreamTxCharacteristic->getValue().size());
                ESP_LOGD(Tag, "Pacote enviado: %s", bleservice->pStreamTxCharacteristic->getValue().c_str());
            }

            cJSON_Delete(StreamData);
            cJSON_free(json_Data);
        }
    }

}

BLEServerService::BLEServerService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{

    ESP_LOGD(this->GetName().c_str(), "Iniciando fila");

    queuePacketsReceived = xQueueCreate(10, sizeof(ble_gatt_uart_packet_t));

    ESP_LOGD(this->GetName().c_str(), "Iniciando servidor GATT...");

    NimBLEDevice::init(Robot::getInstance()->GetName());
    NimBLEDevice::setPower(esp_power_level_t::ESP_PWR_LVL_P9, esp_ble_power_type_t::ESP_BLE_PWR_TYPE_ADV);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UART_UUID);

    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,
                                                       NIMBLE_PROPERTY::NOTIFY);

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,
                                                                          NIMBLE_PROPERTY::WRITE);

    pRxCharacteristic->setCallbacks(new CharacteristicCallbacks());

    pService->start();

    BLEService *pStreamService = pServer->createService(SERVICE_STREAM_UUID);
    pStreamTxCharacteristic = pStreamService->createCharacteristic(CHARACTERISTIC_STREAM_TX,
                                                                    NIMBLE_PROPERTY::NOTIFY);

    pStreamService->start();

    pServer->getAdvertising()->start();


    ESP_LOGD(this->GetName().c_str(), "Começou a se anunciar...");
}

void BLEServerService::Run()
{
    
    ESP_LOGD(this->GetName().c_str(), "Criando Task para Stream de dados...");
    xTaskCreate(RunStream, "TaskStream", 8192, NULL, 18, &xTaskStream);

    ESP_LOGD(this->GetName().c_str(), "Iniciando Loop...");

    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queuePacketsReceived, &packetReceived, portMAX_DELAY);

        ESP_LOGD(GetName().c_str(), "Recebido pacote de dados");
        ESP_LOGD(GetName().c_str(), "MTU do client: %d", pServer->getPeerInfo(packetReceived.clientMAC).getMTU());
        ESP_LOGD(GetName().c_str(), "Tamanho do pacote: %d", packetReceived.len);
        ESP_LOGD(GetName().c_str(), "Dados:\n%s\n", packetReceived.payload);

        std::string ret;

        ESP_LOGD(GetName().c_str(), "Comando recebido via BLE GATT: %s", packetReceived.payload);

        esp_err_t err = better_console_run((char *)packetReceived.payload, &ret);

        ESP_LOGD(GetName().c_str(), "Retorno do comando:\n%s\nRetorno da execução: %s\n", ret.c_str(), esp_err_to_name(err));

        if (err == ESP_OK)
        {
            cJSON *root;
            root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "cmdExecd", (char *)packetReceived.payload);
            cJSON_AddStringToObject(root, "data", ret.c_str());

            char *my_json_string = cJSON_PrintUnformatted(root);

            ESP_LOGD(GetName().c_str(), "Enviando pacote de dados, tamanho: %d", strlen(my_json_string));

            // Enviar pacote de dados para o cliente via BLE GATT chunks do tamanho do MTU do cliente
            size_t msgSize = strlen(my_json_string) + 1; // +1 no final para pegar /0
            const uint16_t clientMTU = pServer->getPeerInfo(packetReceived.clientMAC).getMTU() * 0.85;

            for (size_t i = 0; msgSize > 0; i += clientMTU, msgSize -= pTxCharacteristic->getValue().size())
            {
                pTxCharacteristic->setValue((uint8_t *)&my_json_string[i], msgSize < clientMTU ? msgSize : clientMTU);
                pTxCharacteristic->notify();
                ESP_LOGD(GetName().c_str(), "Pacote enviado, tamanho: %d", pTxCharacteristic->getValue().size());
            }

            // pTxCharacteristic->setValue(std::string(my_json_string));
            // pTxCharacteristic->notify();

            cJSON_Delete(root);
            cJSON_free(my_json_string);
        }

        free((void *)packetReceived.payload);
    }
}

#endif