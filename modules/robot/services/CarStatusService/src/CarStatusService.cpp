#include "CarStatusService.hpp"

QueueHandle_t CarStatusService::gpio_evt_queue;

void IRAM_ATTR CarStatusService::gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    uint8_t carstate = 0;
    if (gpio_num == GPIO_NUM_0)
    {
        carstate = CAR_IN_LINE;
    }
    xQueueSendFromISR(gpio_evt_queue, &carstate, NULL);
}

CarStatusService::CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->latMarks = robot->getSLatMarks();
    this->PidTrans = robot->getPIDVel();

    mappingService = MappingService::getInstance();

    if(!status->TunningMode->getData())
    {
        latMarks->marks->loadData();

        if (latMarks->marks->getSize() <= 0)
        {
            status->encreading->setData(false);
            status->robotIsMapping->setData(true);
        }
        else
        {
            status->robotIsMapping->setData(false);
            status->encreading->setData(true);
            numMarks = latMarks->marks->getSize();
            mediaEncFinal = latMarks->marks->getData(numMarks - 1).MapEncMedia;
        }
    }
    status->robotState->setData(CAR_STOPPED);

    stateChanged = true;
    lastMappingState = false;
    lastState = status->robotState->getData();
    lastTrack = (TrackState) status->TrackStatus->getData();

    firstmark = false;

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_0);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(GPIO_NUM_0, gpio_isr_handler, (void *)GPIO_NUM_0);
}

void CarStatusService::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // int iloop = 0;

    ESP_LOGD(GetName().c_str(), "Aguardando pressionamento do botão.");

    uint8_t num;
    do
    {
        xQueueReceive(gpio_evt_queue, &num, portMAX_DELAY);
        ESP_LOGD(GetName().c_str(), "Aguardando inicialização");
        if(status->robotState->getData() != CAR_STOPPED) break;
    } while (num != CAR_IN_LINE);

    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    command.color = LED_COLOR_RED;
    LEDsService::getInstance()->queueCommand(command);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    // Deletar o mapeamento caso o botão de boot seja mantido pressionado e exista mapeamento na flash
    if(!gpio_get_level(GPIO_NUM_0) && latMarks->marks->getSize() > 0 && !status->TunningMode->getData() && status->HardDeleteMap->getData())
    {
        DataStorage::getInstance()->delete_data("sLatMarks.marks");
        status->encreading->setData(false);
        status->robotIsMapping->setData(true);
        ESP_LOGD(GetName().c_str(), "Mapeamento Deletado");
        command.color = LED_COLOR_YELLOW;
        LEDsService::getInstance()->queueCommand(command);
    }
    ESP_LOGD(GetName().c_str(), "Iniciando delay de 1500ms");
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (status->robotIsMapping->getData() && !status->TunningMode->getData())
    {
        ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");
        command.color = LED_COLOR_YELLOW;
        LEDsService::getInstance()->queueCommand(command);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // Começa mapeamento
        mappingService->startNewMapping();
    }

    if(!status->TunningMode->getData())
    {
        status->robotState->setData(CAR_IN_CURVE);
        started_in_Tuning = false;
    }
    else
    {
        started_in_Tuning = true;
        status->robotState->setData(CAR_TUNING);
        status->encreading->setData(false);
        status->robotIsMapping->setData(false);
        latMarks->marks->clearAllData();
        numMarks = 0;
        mediaEncFinal = 0;
        command.led[0] = LED_POSITION_FRONT;
        command.led[1] = LED_POSITION_NONE;
        command.color = LED_COLOR_WHITE;
        command.effect = LED_EFFECT_SET;
        command.brightness = 0.5;
        LEDsService::getInstance()->queueCommand(command);        
    }
    status->FirstMark->setData(false);
    // Loop
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
        
        status->stateMutex.lock();
        TrackLen = (TrackState)status->TrackStatus->getData();
        pulsesBeforeCurve = latMarks->PulsesBeforeCurve->getData();
        pulsesAfterCurve = latMarks->PulsesAfterCurve->getData();
        actualCarState = (CarState) status->robotState->getData();
        if(started_in_Tuning && status->TunningMode->getData() && status->robotState->getData() != CAR_TUNING &&  !status->encreading->getData() && !status->robotIsMapping->getData()) 
        {
            status->robotState->setData(CAR_TUNING);
            command.led[0] = LED_POSITION_FRONT;
            command.led[1] = LED_POSITION_NONE;
            command.color = LED_COLOR_WHITE;
            command.effect = LED_EFFECT_SET;
            command.brightness = 0.5;
            LEDsService::getInstance()->queueCommand(command);  
        }
        if(latMarks->rightMarks->getData() >= 1 && !firstmark)
        {
            firstmark = true;
            status->FirstMark->setData(true);
            initialmediaEnc = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2;
        }

        if (lastMappingState != status->robotIsMapping->getData() && status->robotIsMapping->getData())
        {
            lastMappingState = status->robotIsMapping->getData();

            ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo mapeamento.");
            command.color = LED_COLOR_YELLOW;
            LEDsService::getInstance()->queueCommand(command);
        }

        else if ((lastState != status->robotState->getData() || lastTrack != (TrackState)status->TrackStatus->getData()) && !lastMappingState && status->robotState->getData() != CAR_STOPPED && status->robotState->getData() != CAR_TUNING)
        {
            lastState = status->robotState->getData();
            lastTrack =  (TrackState)status->TrackStatus->getData();
            if (lastState == CAR_IN_LINE)
            {
                ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo inLine.");
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_GREEN;
                command.effect = LED_EFFECT_SET;
                switch (TrackLen)
                {
                    case SHORT_LINE:
                        command.brightness = 0.05;
                        break;
                    case MEDIUM_LINE:
                        command.brightness = 0.3;
                        break;
                    case LONG_LINE:
                        command.brightness = 1;
                        break;
                    default:
                        command.brightness = 1;
                        break;
                }
                LEDsService::getInstance()->queueCommand(command);
            }
            else
            {
                ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo inCurve.");
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_RED;
                command.effect = LED_EFFECT_SET;
                switch (TrackLen)
                {
                    case SHORT_CURVE:
                        command.brightness = 0.05;
                        break;
                    case MEDIUM_CURVE:
                        command.brightness = 0.3;
                        break;
                    case LONG_CURVE:
                        command.brightness = 1;
                        break;
                    default:
                        command.brightness = 1;
                        break;
                }
                LEDsService::getInstance()->queueCommand(command);
            }
        }

        mediaEncActual = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2; // calcula media dos encoders

//         if (iloop >= 20 && !status->robotIsMapping->getData())
//         {
//             ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
//             ESP_LOGD(GetName().c_str(), "initialEncMedia: %d", initialmediaEnc);
//             ESP_LOGD(GetName().c_str(), "EncMedia: %d", mediaEncActual);
//             ESP_LOGD(GetName().c_str(), "EncMediaoffset: %d", mediaEncActual-initialmediaEnc);
//             ESP_LOGD(GetName().c_str(), "mediaEncFinal: %d", mediaEncFinal);
//             ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PidTrans->setpoint->getData());
//             iloop = 0;
//         }
//         iloop++;

        if(!status->robotIsMapping->getData() && !status->encreading->getData() && !status->TunningMode->getData() && actualCarState != CAR_STOPPED){
            robot->getStatus()->robotState->setData(CAR_STOPPED);
            DataManager::getInstance()->saveAllParamDataChanged();
            command.led[0] = LED_POSITION_FRONT;
            command.led[1] = LED_POSITION_NONE;
            command.color = LED_COLOR_BLACK;
            command.effect = LED_EFFECT_SET;
            command.brightness = 1;
            LEDsService::getInstance()->queueCommand(command);
        }

        if (!status->robotIsMapping->getData() && actualCarState != CAR_STOPPED && status->encreading->getData() && firstmark && (!status->TunningMode->getData() || !started_in_Tuning))
        {
            if ((mediaEncActual - initialmediaEnc) >= mediaEncFinal)
            {
                ESP_LOGD(GetName().c_str(), "Parando o robô");
                status->encreading->setData(false);
                //vTaskDelay(100 / portTICK_PERIOD_MS);

                // TODO: Encontrar forma bonita de suspender os outros serviços.
                // vTaskSuspend(xTaskPID);
                // vTaskSuspend(xTaskSensors);

                robot->getStatus()->robotState->setData(CAR_STOPPED);
                DataManager::getInstance()->saveAllParamDataChanged();
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_BLACK;
                command.effect = LED_EFFECT_SET;
                command.brightness = 1;
                LEDsService::getInstance()->queueCommand(command);
            }
            if ((mediaEncActual - initialmediaEnc) < mediaEncFinal)
            {
                // define o status do carrinho se o mapeamento não estiver ocorrendo
                int mark = 0;
                for (mark = 0; mark < numMarks - 1; mark++)
                {
                    // Verifica a contagem do encoder e atribui o estado ao robô
                    int32_t Manualmedia = latMarks->marks->getData(mark).MapEncMedia;        // Média dos encoders na chave mark
                    int32_t ManualmediaNxt = latMarks->marks->getData(mark + 1).MapEncMedia; // Média dos encoders na chave mark + 1

                    if ((mediaEncActual - initialmediaEnc) >= Manualmedia && (mediaEncActual - initialmediaEnc) <= ManualmediaNxt) // análise do valor das médias dos encoders
                    {
                        CarState trackType = (CarState)latMarks->marks->getData(mark+1).MapStatus;
                        TrackState trackLen = (TrackState)latMarks->marks->getData(mark+1).MapTrackStatus;
                        // Verifica se o robô precisa reduzir a velocidade, entrando no modo curva
                        if((CarState)latMarks->marks->getData(mark).MapStatus == CAR_IN_CURVE && (CarState)latMarks->marks->getData(mark + 1).MapStatus == CAR_IN_LINE)
                        {
                            if((Manualmedia + pulsesAfterCurve) < ManualmediaNxt && (mediaEncActual - initialmediaEnc) < (Manualmedia + pulsesAfterCurve)) 
                            {
                                trackType = CAR_IN_CURVE;
                                trackLen = (TrackState)latMarks->marks->getData(mark).MapTrackStatus;
                            }
                            else if((Manualmedia + pulsesAfterCurve) >= ManualmediaNxt) 
                            {
                                trackType = CAR_IN_CURVE;
                                trackLen = (TrackState)latMarks->marks->getData(mark).MapTrackStatus;
                            }
                        }
                        if(mark + 2 < numMarks)
                        {
                            if((CarState)latMarks->marks->getData(mark+1).MapStatus == CAR_IN_LINE && (CarState)latMarks->marks->getData(mark + 2).MapStatus == CAR_IN_CURVE)
                            {
                                if((ManualmediaNxt - pulsesBeforeCurve) > Manualmedia && (mediaEncActual - initialmediaEnc) > (ManualmediaNxt - pulsesBeforeCurve)) 
                                {
                                    trackType = CAR_IN_CURVE;
                                    trackLen = (TrackState)latMarks->marks->getData(mark+2).MapTrackStatus;
                                }
                                else if((ManualmediaNxt - pulsesBeforeCurve) <= Manualmedia) 
                                {
                                    trackType = CAR_IN_CURVE;
                                    trackLen = (TrackState)latMarks->marks->getData(mark+2).MapTrackStatus;
                                }
                            }
                        }
                        // Atualiza estado do robô
                        status->robotState->setData(trackType);
                        status->TrackStatus->setData(trackLen);
                        break;
                    }
                }
            }
        }

        status->stateMutex.unlock();
    }
}