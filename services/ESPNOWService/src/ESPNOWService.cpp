#include "ESPNOWService.hpp"

ESPNOWService::ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    status = robot->getStatus();
    protocolHandler.EspNowInit(1, broadcastAddress, false);
};

void ESPNOWService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // Analisa os comandos recebidos em texto
        std::string cmd="";
        cmd = msgrecebida;
        struct CarParameters ReceivedParams;
        if(cmd.find("stop") != -1 && cmd.size() > 0){
            status->setState(CAR_STOPPED);
            strcpy(msgrecebida,"empty");
        }
        else if(cmd.find("map") != -1 && cmd.size() > 0){
            robot->getSLatMarks()->SetMapFinished(false);
            robot->getSLatMarks()->SetrightMarks(0);
            status->setState(CAR_IN_LINE);
            status->setMapping(true);
            robot->Setparams();
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
                switch (packetReceive.cmd) // Prepara pacotes recebidos
                {
                    case MapDataSend:
                        struct SLatMarks LatMarks;
                        memcpy(&LatMarks,dataReceived,packetReceive.size);
                        robot->getSLatMarks()->setData(LatMarks);
                        status->setMapping(false);
                        robot->getSLatMarks()->SetrightMarks(0);
                        status->setState(CAR_IN_LINE);
                        robot->Setparams();
                        ESP_LOGD(GetName().c_str(),"Comando Recebido: Encoders iniciados");
                        break;
                    case CMDTXT:
                        memcpy(msgrecebida,dataReceived,packetReceive.size);
                        ESP_LOGD(GetName().c_str(),"Comando Recebido: %s",msgrecebida);
                        break;
                    case ParametersSend:
                        memcpy(&ReceivedParams,dataReceived,packetReceive.size);
                        ParamsData = robot->GetParams();
                        if(packetReceive.version == 1){
                            ParamsData.KpRotMapLine = ReceivedParams.KpRotMapLine;
                            ParamsData.KpRotMapCurve = ReceivedParams.KpRotMapCurve;
                            ParamsData.KpRotRunLine = ReceivedParams.KpRotRunLine;
                            ParamsData.KpRotRunCurve = ReceivedParams.KpRotRunCurve;
                            ParamsData.KdRotMapLine = ReceivedParams.KdRotMapLine;
                            ParamsData.KdRotMapCurve = ReceivedParams.KdRotMapCurve;
                            ParamsData.KdRotRunLine = ReceivedParams.KdRotRunLine;
                            ParamsData.KdRotRunCurve = ReceivedParams.KdRotRunCurve;
                        }
                        else if(packetReceive.version == 2){
                            ParamsData.KpVelMapLine = ReceivedParams.KpVelMapLine;
                            ParamsData.KpVelMapCurve = ReceivedParams.KpVelMapCurve;
                            ParamsData.KpVelRunLine = ReceivedParams.KpVelRunLine;
                            ParamsData.KpVelRunCurve = ReceivedParams.KpVelRunCurve;
                            ParamsData.KdVelMapLine = ReceivedParams.KdVelMapLine;
                            ParamsData.KdVelMapCurve = ReceivedParams.KdVelMapCurve;
                            ParamsData.KdVelRunLine = ReceivedParams.KdVelRunLine;
                            ParamsData.KdVelRunCurve = ReceivedParams.KdVelRunCurve;
                        }
                        else if(packetReceive.version == 3){
                            ParamsData.SpeedMinMapLine = ReceivedParams.SpeedMinMapLine;
                            ParamsData.SpeedMinMapCurve = ReceivedParams.SpeedMinMapCurve;
                            ParamsData.SpeedBaseMapLine = ReceivedParams.SpeedBaseMapLine;
                            ParamsData.SpeedBaseMapCurve = ReceivedParams.SpeedBaseMapCurve;
                            ParamsData.SpeedMaxMapLine = ReceivedParams.SpeedMaxMapLine;
                            ParamsData.SpeedMaxMapCurve = ReceivedParams.SpeedMaxMapCurve;
                            ParamsData.VelTargetMap = ReceivedParams.VelTargetMap;
                        }
                        else if(packetReceive.version == 4){
                            ParamsData.SpeedMinRunLine = ReceivedParams.SpeedMinRunLine;
                            ParamsData.SpeedMinRunCurve = ReceivedParams.SpeedMinRunCurve;
                            ParamsData.SpeedBaseRunLine = ReceivedParams.SpeedBaseRunLine;
                            ParamsData.SpeedBaseRunCurve = ReceivedParams.SpeedBaseRunCurve;
                            ParamsData.SpeedMaxRunLine = ReceivedParams.SpeedMaxRunLine;
                            ParamsData.SpeedMaxRunCurve = ReceivedParams.SpeedMaxRunCurve;
                            ParamsData.VelTargetRunLine = ReceivedParams.VelTargetRunLine;
                            ParamsData.VelTargetRunCurve = ReceivedParams.VelTargetRunCurve;
                        }
                        robot->Setparams(ParamsData);
                        ESP_LOGD(GetName().c_str(),"Parâmetros Recebidos");
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