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
            initialRobotState = CAR_MAPPING;
        }
        else
        {
            initialRobotState = CAR_ENC_READING;
            numMarks = latMarks->marks->getSize();
            mediaEncFinal = latMarks->marks->getData(numMarks - 1).MapEncMedia;
        }
    }
    status->robotState->setData(CAR_STOPPED);
    status->RealTrackStatus->setData(UNDEFINED);
    status->TrackStatus->setData(UNDEFINED);

    lastPaused = status->robotPaused->getData();
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

    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_RED, 1);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    // Deletar o mapeamento caso o botão de boot seja mantido pressionado e exista mapeamento na flash
    if(!gpio_get_level(GPIO_NUM_0) && latMarks->marks->getSize() > 0 && !status->TunningMode->getData() && status->HardDeleteMap->getData())
    {
        DataStorage::getInstance()->delete_data("sLatMarks.marks");
        initialRobotState = CAR_MAPPING;
        ESP_LOGD(GetName().c_str(), "Mapeamento Deletado");
        LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
    }
    ESP_LOGD(GetName().c_str(), "Iniciando delay de 1500ms");
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (initialRobotState == CAR_MAPPING && !status->TunningMode->getData())
    {
        ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");
        LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // Começa mapeamento
        status->RealTrackStatus->setData(UNDEFINED);
        status->TrackStatus->setData(UNDEFINED);
        mappingService->startNewMapping();
    }

    if(!status->TunningMode->getData())
    {
        if(initialRobotState != CAR_MAPPING)
        {
            status->TrackStatus->setData(SHORT_LINE);
            status->RealTrackStatus->setData(SHORT_LINE);
        }
        started_in_Tuning = false;
    }
    else
    {
        started_in_Tuning = true;
        initialRobotState = CAR_TUNING;
        status->TrackStatus->setData(TUNING);
        status->RealTrackStatus->setData(TUNING);
        latMarks->marks->clearAllData();
        numMarks = 0;
        mediaEncFinal = 0;
        LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_WHITE, 0.5);       
    }
    status->FirstMark->setData(false);
    status->robotState->setData(initialRobotState);
    // Loop
    for (;;)
    {
        
        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
        status->stateMutex.lock();
        TrackLen = (TrackState)status->TrackStatus->getData();
        pulsesBeforeCurve = latMarks->PulsesBeforeCurve->getData();
        pulsesAfterCurve = latMarks->PulsesAfterCurve->getData();
        actualCarState = (CarState) status->robotState->getData();
        if(status->robotPaused->getData()) lastPaused = true;
        if(latMarks->rightMarks->getData() >= 1 && !firstmark)
        {
            firstmark = true;
            status->FirstMark->setData(true);
            initialmediaEnc = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2;
        }

        else if ((lastState != status->robotState->getData() || lastTrack != (TrackState)status->TrackStatus->getData() || lastTransition != status->Transition->getData() || (lastPaused && !status->robotPaused->getData())) && status->robotState->getData() != CAR_STOPPED)
        {
            lastPaused = false;
            lastState = status->robotState->getData();
            lastTrack =  (TrackState)status->TrackStatus->getData();
            lastTransition = status->Transition->getData();
            if (!status->car_in_curve(lastTrack) && !lastTransition && lastState == CAR_ENC_READING)
            {
                ESP_LOGD(GetName().c_str(), "Alterando os leds para modo inLine.");
                led_color_t color = LED_COLOR_GREEN;
                float brightness = 1;
                switch (TrackLen)
                {
                    case SHORT_LINE:
                        brightness = 0.05;
                        break;
                    case MEDIUM_LINE:
                        brightness = 0.3;
                        break;
                    case LONG_LINE:
                        brightness = 1;
                        break;
                    case XLONG_LINE:
                        brightness = 1;
                        break;
                    case SPECIAL_TRACK:
                        color = LED_COLOR_PURPLE;
                        brightness = 0.05;
                        break;
                    default:
                        color = LED_COLOR_WHITE;
                        brightness = 1;
                        break;
                }
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, brightness);
            }
            else if(status->car_in_curve(lastTrack) && !lastTransition && lastState == CAR_ENC_READING)
            {
                ESP_LOGD(GetName().c_str(), "Alterando os leds para modo inCurve.");
                led_color_t color = LED_COLOR_RED;
                float brightness = 1;
                switch (TrackLen)
                {
                    case XLONG_CURVE:
                        brightness = 1;
                        break;
                    case SHORT_CURVE:
                        brightness = 0.05;
                        break;
                    case MEDIUM_CURVE:
                        brightness = 0.3;
                        break;
                    case LONG_CURVE:
                        brightness = 1;
                        break;
                    case ZIGZAG:
                        color = LED_COLOR_PURPLE;
                        brightness = 1;
                        break;
                    case SPECIAL_TRACK:
                        color = LED_COLOR_PURPLE;
                        brightness = 0.05;
                        break;
                    default:
                        color = LED_COLOR_WHITE;
                        brightness = 1;
                        break;
                }
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, brightness);
            }
            else if(lastTransition && lastState == CAR_ENC_READING)
            {
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLUE, 0.5);
            }
            else if(lastState == CAR_TUNING)
            {
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_WHITE, 0.5);  
            }
            else if(lastState == CAR_MAPPING)
            {
                ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo mapeamento.");
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 0.5); 
            }
        }

        mediaEncActual = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2; // calcula media dos encoders

         if (iloop >= 30)
         {
            ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
            ESP_LOGD(GetName().c_str(), "initialEncMedia: %ld", initialmediaEnc);
            ESP_LOGD(GetName().c_str(), "EncMedia: %ld", mediaEncActual);
            ESP_LOGD(GetName().c_str(), "EncMediaoffset: %ld", mediaEncActual-initialmediaEnc);
            ESP_LOGD(GetName().c_str(), "mediaEncFinal: %ld", mediaEncFinal);
            ESP_LOGD(GetName().c_str(), "Speed: %.2f", speed->CalculatedSpeed->getData());
            iloop = 0;
         }
         iloop++;

        if(actualCarState == CAR_TUNING && !status->TunningMode->getData()){
            robot->getStatus()->robotState->setData(CAR_STOPPED);
            vTaskDelay(0);
            DataManager::getInstance()->saveAllParamDataChanged();
            LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
        }

        if (actualCarState == CAR_ENC_READING && firstmark && (!status->TunningMode->getData() || !started_in_Tuning))
        {
            if ((mediaEncActual - initialmediaEnc) >= mediaEncFinal)
            {
                if(status->TuningMapped->getData())
                {
                    status->FirstMark->setData(false);
                    firstmark = false;
                    initialmediaEnc = 0;
                    status->robotState->setData(CAR_ENC_READING);
                    status->TrackStatus->setData(SHORT_LINE);
                    status->RealTrackStatus->setData(SHORT_LINE);
                    latMarks->rightMarks->setData(0);
                }
                else
                {
                    ESP_LOGD(GetName().c_str(), "Parando o robô");
                    //vTaskDelay(100 / portTICK_PERIOD_MS);

                    // TODO: Encontrar forma bonita de suspender os outros serviços.
                    // vTaskSuspend(xTaskPID);
                    // vTaskSuspend(xTaskSensors);

                    robot->getStatus()->robotState->setData(CAR_STOPPED);
                    DataManager::getInstance()->saveAllParamDataChanged();
                    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
                }
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
                        TrackState trackLen = (TrackState)latMarks->marks->getData(mark+1).MapTrackStatus;
                        status->RealTrackStatus->setData(trackLen);
                        bool transition = false;

                        int16_t offset = latMarks->marks->getData(mark).MapOffset;
                        int16_t offsetnxt = latMarks->marks->getData(mark+1).MapOffset;
                        // Verifica se o robô precisa reduzir a velocidade, entrando no modo curva
                        if(status->car_in_curve((TrackState)latMarks->marks->getData(mark).MapTrackStatus) && !status->car_in_curve((TrackState)latMarks->marks->getData(mark + 1).MapTrackStatus) && offset == 0)
                        {
                            offset = pulsesAfterCurve; 
                        }
                        if(offset > 0)
                        {
                            if((mediaEncActual - initialmediaEnc) < (Manualmedia + offset)) 
                            {
                                transition = true;
                                trackLen = (TrackState)latMarks->marks->getData(mark).MapTrackStatus;
                            }
                        }
                        if(mark + 2 < numMarks)
                        {
                            if(!status->car_in_curve((TrackState)latMarks->marks->getData(mark+1).MapTrackStatus) && status->car_in_curve((TrackState)latMarks->marks->getData(mark + 2).MapTrackStatus) && offsetnxt == 0)
                            {
                                offsetnxt = -pulsesBeforeCurve; 
                            }
                            if(offsetnxt < 0)
                            {
                                if((mediaEncActual - initialmediaEnc) > (ManualmediaNxt + offsetnxt)) 
                                {
                                    transition = true;
                                    trackLen = (TrackState)latMarks->marks->getData(mark+2).MapTrackStatus;
                                }
                            }
                        }
                        // Atualiza estado do robô
                        status->Transition->setData(transition);
                        status->TrackStatus->setData(trackLen);
                        break;
                    }
                }
            }
        }

        status->stateMutex.unlock();
    }
}