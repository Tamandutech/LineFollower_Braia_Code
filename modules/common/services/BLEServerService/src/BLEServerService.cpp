#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED)

#include "BLEServerService.hpp"

QueueHandle_t BLEServerService::queuePacketsReceived;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client conectado.");
        BLEServerService::getInstance()->deviceConnected = true;
        printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 3x interval time for best results.
         */
       // pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 18);
    };

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(), "Client disconnected - start advertising");
        BLEServerService::getInstance()->deviceConnected = false;
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) {
        printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
        //pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
    };

/********************* Security handled here **********************
****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
        printf("Server Passkey Request\n");
        /** This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key){
        printf("The passkey YES/NO number: %" PRIu32"\n", pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(NimBLEConnInfo& connInfo){
        /** Check that encryption was successful, if not we disconnect the client */
        if(!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            printf("Encrypt connection failed - disconnecting client\n");
            return;
        }
        printf("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        printf("%s : onRead(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        ble_gatt_uart_packet_t tempPacket;

        tempPacket.len = pCharacteristic->getValue().length();
        tempPacket.payload = (uint8_t *)malloc(tempPacket.len + 1);
        tempPacket.clientMAC = connInfo.getIdAddress();

        strcpy((char *)tempPacket.payload, pCharacteristic->getValue().c_str());

        if (pCharacteristic->getUUID() == NimBLEUUID(CHARACTERISTIC_UUID_RX))
        {
            xQueueSend(BLEServerService::queuePacketsReceived, &tempPacket, 0);
        }
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"%s : onWrite(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"Sending notification to clients\n");
    }

    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"Notification/Indication return code: %d, %s\n",
               code, NimBLEUtils::returnCodeToString(code));
    }

    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        std::string str = "Client ID: ";
        str += connInfo.getConnHandle();
        str += " Address: ";
        str += connInfo.getAddress().toString();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());

        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"%s\n", str.c_str());
    }
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
        std::string dscVal = pDescriptor->getValue();
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"Descriptor witten value: %s\n", dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
        ESP_LOGD(BLEServerService::getInstance()->GetName().c_str(),"%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
    };
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

    NimBLEService *pService = pServer->createService(SERVICE_UART_UUID);

    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,
                                                       NIMBLE_PROPERTY::NOTIFY);

    NimBLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,
                                                                          NIMBLE_PROPERTY::WRITE);

    pRxCharacteristic->setCallbacks(&chrCallbacks);

    pService->start();

    NimBLEService *pStreamService = pServer->createService(SERVICE_STREAM_UUID);
    pStreamTxCharacteristic = pStreamService->createCharacteristic(CHARACTERISTIC_STREAM_TX,
                                                                    NIMBLE_PROPERTY::NOTIFY);

    pStreamService->start();

    NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->addServiceUUID(pStreamService->getUUID());
    
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

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
        ESP_LOGD(GetName().c_str(), "MTU do client: %u", pServer->getPeerInfo(packetReceived.clientMAC).getMTU());
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