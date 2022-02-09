#include "ESPNOWService.hpp"

ESPNOWService::ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    protocolHandler.EspNowInit(1, broadcastAddress, false);
};

void ESPNOWService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // Analisa os comandos recebidos em texto
        if(strcmp(msgrecebida,"stop") == 0 && strlen(msgrecebida)!=0){
            robot->getStatus()->setState(CAR_STOPPED);
            strcpy(msgrecebida,"empty");
        }
        else if(strcmp(msgrecebida,"start") == 0 && strlen(msgrecebida)!=0){
            strcpy(msgrecebida,"empty");
        }
        else if(strcmp(msgrecebida,"startMap") == 0 && strlen(msgrecebida)!=0){
            strcpy(msgrecebida,"empty");
        }
        if (protocolHandler.dataAvailable())
        {
            packetReceive = protocolHandler.getPacketReceived();
            if(!newData) { // Prepara o ponteiro para o recebimento dos dados nos pacotes
                dataReceived = (uint8_t *) malloc(packetReceive.size);
                ptrPos = 0;
                newData = true;
            }
            if(packetReceive.packetsToReceive==0){ // Executa as tarefas relacionados com o último comando recebido
                newData = false;
                memcpy(dataReceived+ptrPos,packetReceive.data,packetReceive.packetsize);
                switch (packetReceive.cmd)
                {
                    case MapDataSend:
                        struct SLatMarks LatMarks;
                        memcpy(&LatMarks,dataReceived,packetReceive.size);
                        break;
                    case CMDTXT:
                        memcpy(msgrecebida,dataReceived,packetReceive.size);
                        ESP_LOGD(GetName().c_str(),"Comando Recebido: %s",msgrecebida);
                        break;
                    default:
                        ESP_LOGD(GetName().c_str(),"Comando inválido");
                        break;
                }
                free(dataReceived);   
            }
            else{ // Adiciona os dados dos pacotes no ponteiro
                memcpy(dataReceived+ptrPos,packetReceive.data,packetReceive.packetsize);
                ptrPos+=packetReceive.packetsize;
            }

        }
        if(robot->PacketSendavailable()){ // Verifica se há pacotes para o envio
            packetSend = robot->getPacketSend();
            uint8_t *datatoSend = (uint8_t *)malloc(packetSend.size);
            switch (packetSend.cmd) // Prepara os pacotes para serem enviados
            {
            case MapDataSend:
                struct SLatMarks LatMarksData;
                LatMarksData = robot->getSLatMarks()->getData();
                memcpy(datatoSend,&LatMarksData,packetSend.size);
                break;
            case MarkData:
                memcpy(datatoSend,packetSend.data,packetSend.size);
                break; 
            default:
                break;
            }
            esp_err_t erro = protocolHandler.EspSend(packetSend.cmd, packetSend.version, packetSend.size, datatoSend);
            char erroMsg[150];
            ESP_LOGE(GetName().c_str(),"Error: %d",erro);
            esp_err_to_name_r(erro,erroMsg,150);
            ESP_LOGE(GetName().c_str(),"Error Name: %s",erroMsg);
            free(datatoSend);
        }
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}