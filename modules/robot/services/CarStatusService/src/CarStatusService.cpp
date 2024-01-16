#include "CarStatusService.hpp"

SemaphoreHandle_t CarStatusService::SemaphoreButton;

void IRAM_ATTR CarStatusService::startRobotWithBootButton(void *arg)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreButton, &high_task_awoken);
}

CarStatusService::CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->latMarks = robot->getSLatMarks();

    mappingService = MappingService::getInstance();

    status->robotState->setData(CAR_STOPPED);
    status->RealTrackStatus->setData(DEFAULT_TRACK);
    status->TrackStatus->setData(DEFAULT_TRACK);

    lastPaused = status->robotPaused->getData();
    lastState = status->robotState->getData();
    lastTrack = (TrackSegment)status->TrackStatus->getData();

    SemaphoreButton = xSemaphoreCreateBinary();
    configExternInterruptToReadButton(GPIO_NUM_0);

    firstmark = false;

    if (!status->TunningMode->getData())
        defineIfRobotWillStartMappingMode();
}

void CarStatusService::defineIfRobotWillStartMappingMode()
{
    latMarks->marks->loadData();
    initialRobotState = CAR_MAPPING;
    if (latMarks->marks->getSize() > 0)
        startFollowingDefinedMapping();
}

void CarStatusService::startFollowingDefinedMapping()
{
    status->TrackStatus->setData(SHORT_LINE);
    status->RealTrackStatus->setData(SHORT_LINE);

    initialRobotState = CAR_ENC_READING_BEFORE_FIRSTMARK;
    numMarks = latMarks->marks->getSize();
    mediaEncFinal = latMarks->marks->getData(numMarks - 1).MapEncMedia;
}

void CarStatusService::configExternInterruptToReadButton(gpio_num_t interruptPort)
{
    gpio_config_t interruptConfig = {};
    interruptConfig.intr_type = GPIO_INTR_NEGEDGE;
    interruptConfig.pin_bit_mask = (1ULL << interruptPort);
    interruptConfig.mode = GPIO_MODE_INPUT;
    interruptConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    interruptConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_config(&interruptConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(interruptPort, startRobotWithBootButton, NULL);
}

void CarStatusService::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    waitPressBootButtonToStart();

    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_RED, 1);

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (!gpio_get_level(GPIO_NUM_0) && latMarks->marks->getSize() > 0 && !status->TunningMode->getData() && status->HardDeleteMap->getData())
        deleteMappingIfBootButtonIsPressed();

    ESP_LOGD(GetName().c_str(), "Iniciando delay de 1500ms");
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    
    if (initialRobotState == CAR_MAPPING && !status->TunningMode->getData())
        startMappingTheTrack();

    started_in_Tuning = false;   
    if (status->TunningMode->getData())
        setTuningMode();
        

    status->robotState->setData(initialRobotState);
    
    // Loop
    for (;;)
    {

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
        TrackLen = (TrackSegment)status->TrackStatus->getData();
        pulsesBeforeCurve = latMarks->PulsesBeforeCurve->getData();
        pulsesAfterCurve = latMarks->PulsesAfterCurve->getData();
        actualCarState = (CarState)status->robotState->getData();
        if (status->robotPaused->getData())
            lastPaused = true;
        if (latMarks->rightMarks->getData() >= 1 && !firstmark && actualCarState == CAR_ENC_READING_BEFORE_FIRSTMARK)
        {
            firstmark = true;
            actualCarState = CAR_ENC_READING;
            status->robotState->setData(actualCarState);
            initialmediaEnc = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2;
        }

        if ((lastState != status->robotState->getData() || lastTrack != (TrackSegment)status->TrackStatus->getData() || lastTransition != status->Transition->getData() || (lastPaused && !status->robotPaused->getData())) && status->robotState->getData() != CAR_STOPPED)
        {
            lastPaused = false;
            lastState = status->robotState->getData();
            lastTrack = (TrackSegment)status->TrackStatus->getData();
            lastTransition = status->Transition->getData();

            LedColor color = getStatusColor((CarState)lastState, (TrackSegment)lastTrack);
            float brightness = getSegmentBrightness((CarState)lastState, (TrackSegment)lastTrack);
            if (lastTransition && lastState == CAR_ENC_READING)
            {
                color = LED_COLOR_BLUE;
                brightness = 1;
            }
            LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, brightness);
        }

        mediaEncActual = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2; // calcula media dos encoders

        if (iloop >= 30)
        {
            ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
            ESP_LOGD(GetName().c_str(), "initialEncMedia: %ld", initialmediaEnc);
            ESP_LOGD(GetName().c_str(), "EncMedia: %ld", mediaEncActual);
            ESP_LOGD(GetName().c_str(), "EncMediaoffset: %ld", mediaEncActual - initialmediaEnc);
            ESP_LOGD(GetName().c_str(), "mediaEncFinal: %ld", mediaEncFinal);
            ESP_LOGD(GetName().c_str(), "Speed: %.2f", speed->linearSpeed->getData());
            iloop = 0;
        }
        iloop++;

        if (actualCarState == CAR_TUNING && !status->TunningMode->getData())
        {
            robot->getStatus()->robotState->setData(CAR_STOPPED);
            vTaskDelay(0);
            DataManager::getInstance()->saveAllParamDataChanged();
            LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
        }

        if (actualCarState == CAR_ENC_READING && (!status->TunningMode->getData() || !started_in_Tuning))
        {
            if ((mediaEncActual - initialmediaEnc) >= mediaEncFinal)
            {
                ESP_LOGD(GetName().c_str(), "Parando o robô");

                robot->getStatus()->robotState->setData(CAR_STOPPED);
                DataManager::getInstance()->saveAllParamDataChanged();
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
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
                        TrackSegment trackLen = (TrackSegment)latMarks->marks->getData(mark + 1).MapTrackStatus;
                        status->RealTrackStatus->setData(trackLen);
                        bool transition = false;

                        int16_t offset = latMarks->marks->getData(mark).MapOffset;
                        int16_t offsetnxt = latMarks->marks->getData(mark + 1).MapOffset;
                        // Verifica se o robô precisa reduzir a velocidade, entrando no modo curva
                        if (!isLineSegment((TrackSegment)latMarks->marks->getData(mark).MapTrackStatus) && isLineSegment((TrackSegment)latMarks->marks->getData(mark + 1).MapTrackStatus) && offset == 0)
                        {
                            offset = pulsesAfterCurve;
                        }
                        if (offset > 0)
                        {
                            if ((mediaEncActual - initialmediaEnc) < (Manualmedia + offset))
                            {
                                transition = true;
                                trackLen = (TrackSegment)latMarks->marks->getData(mark).MapTrackStatus;
                            }
                        }
                        if (mark + 2 < numMarks)
                        {

                            if (isLineSegment((TrackSegment)latMarks->marks->getData(mark + 1).MapTrackStatus) && !isLineSegment((TrackSegment)latMarks->marks->getData(mark + 2).MapTrackStatus) && offsetnxt == 0)
                            {
                                offsetnxt = -pulsesBeforeCurve;
                            }
                            if (offsetnxt < 0)
                            {
                                if ((mediaEncActual - initialmediaEnc) > (ManualmediaNxt + offsetnxt))
                                {
                                    transition = true;
                                    trackLen = (TrackSegment)latMarks->marks->getData(mark + 2).MapTrackStatus;
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
    }
}

void CarStatusService::waitPressBootButtonToStart()
{
    ESP_LOGD(GetName().c_str(), "Aguardando pressionamento do botão.");
    xSemaphoreTake(SemaphoreButton, portMAX_DELAY);
}

void CarStatusService::deleteMappingIfBootButtonIsPressed()
{
    DataStorage::getInstance()->delete_data("sLatMarks.marks");
    initialRobotState = CAR_MAPPING;
    ESP_LOGD(GetName().c_str(), "Mapeamento Deletado");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
}

void CarStatusService::startMappingTheTrack() {
     ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");
     LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
     vTaskDelay(1000 / portTICK_PERIOD_MS);
     // Começa mapeamento
     status->RealTrackStatus->setData(DEFAULT_TRACK);
     status->TrackStatus->setData(DEFAULT_TRACK);
     mappingService->startNewMapping();
}

void CarStatusService::setTuningMode()
{
    started_in_Tuning = true;
    initialRobotState = CAR_TUNING;
    latMarks->marks->clearAllData();
    numMarks = 0;
    mediaEncFinal = 0;
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_WHITE, 0.5);
}
