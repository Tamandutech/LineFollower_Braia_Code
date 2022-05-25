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

    // latMarks->marks->loadData();

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

    status->robotState->setData(CAR_STOPPED);

    stateChanged = true;
    lastState = status->robotState->getData();

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
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

    int iloop = 0;

    ESP_LOGD(GetName().c_str(), "Aguardando pressionamento do botão.");

    uint8_t num;
    do
    {
        xQueueReceive(gpio_evt_queue, &num, portMAX_DELAY);
        ESP_LOGD(GetName().c_str(), " Aguardando inicialização");
        if(status->robotState->getData() != CAR_STOPPED) break;
    } while (num != CAR_IN_LINE);

    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.color = LED_COLOR_YELLOW;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    LEDsService::getInstance()->queueCommand(command);
    ESP_LOGD(GetName().c_str(), "Iniciando delay de 2500ms");
    vTaskDelay(2500 / portTICK_PERIOD_MS);
    command.brightness = 0.5;
    command.color = LED_COLOR_GREEN;
    LEDsService::getInstance()->queueCommand(command);

    if (status->robotIsMapping->getData())
    {
        ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");

        // Começa mapeamento
        mappingService->startNewMapping();
    }
    else{
        
    }

    status->robotState->setData(CAR_IN_LINE);

    // Loop
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);

        status->stateMutex.lock();
        if (lastMappingState != status->robotIsMapping->getData() && status->robotIsMapping->getData())
        {
            lastMappingState = status->robotIsMapping->getData();

            ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo mapeamento.");
            speed->setToMapping();
        }

        else if (lastState != status->robotState->getData() && !lastMappingState && status->robotState->getData() != CAR_STOPPED)
        {
            lastState = status->robotState->getData();

            if (lastState == CAR_IN_LINE)
            {
                ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo inLine.");
                speed->setToLine();
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_GREEN;
                command.effect = LED_EFFECT_SET;
                command.brightness = 0.5;
                LEDsService::getInstance()->queueCommand(command);
            }
            else
            {
                ESP_LOGD(GetName().c_str(), "Alterando velocidades para modo inCurve.");
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_RED;
                command.effect = LED_EFFECT_SET;
                command.brightness = 0.5;
                LEDsService::getInstance()->queueCommand(command);
                speed->setToCurve();
            }
        }

        mediaEncActual = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2; // calcula media dos encoders

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
        if (iloop >= 20 && !status->robotIsMapping->getData())
        {
            ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
            ESP_LOGD(GetName().c_str(), "EncMedia: %d", mediaEncActual);
            ESP_LOGD(GetName().c_str(), "mediaEncFinal: %d", mediaEncFinal);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PidTrans->setpoint->getData());
            iloop = 0;
        }
        iloop++;
#endif
        if(!status->robotIsMapping->getData() && !status->encreading->getData()){
            robot->getStatus()->robotState->setData(CAR_STOPPED);
        }

        if (!status->robotIsMapping->getData() && actualCarState != CAR_STOPPED && status->encreading->getData())
        {
            if (mediaEncActual >= mediaEncFinal)
            {
                status->robotState->setData(CAR_IN_LINE);
                vTaskDelay(500 / portTICK_PERIOD_MS);

                // TODO: Encontrar forma bonita de suspender os outros serviços.
                // vTaskSuspend(xTaskPID);
                // vTaskSuspend(xTaskSensors);

                robot->getStatus()->robotState->setData(CAR_STOPPED);
                command.led[0] = LED_POSITION_FRONT;
                command.led[1] = LED_POSITION_NONE;
                command.color = LED_COLOR_BLACK;
                command.effect = LED_EFFECT_SET;
                command.brightness = 1;
                LEDsService::getInstance()->queueCommand(command);
            }
            if (mediaEncActual < mediaEncFinal)
            {
                // define o status do carrinho se o mapeamento não estiver ocorrendo
                int mark = 0;
                for (mark = 0; mark < numMarks - 1; mark++)
                {
                    // Verifica a conGetName().c_str()em do encoder e atribui o estado ao robô

                    int32_t Manualmedia = latMarks->marks->getData(mark).MapEncMedia;        // Média dos encoders na chave mark
                    int32_t ManualmediaNxt = latMarks->marks->getData(mark + 1).MapEncMedia; // Média dos encoders na chave mark + 1

                    if (mediaEncActual >= Manualmedia && mediaEncActual <= ManualmediaNxt)
                    {                                                                                    // análise do valor das médias dos encoders
                        status->robotState->setData((CarState)latMarks->marks->getData(mark).MapStatus); // Atualiza estado do robô
                        break;
                    }
                }
            }
        }

        status->stateMutex.unlock();
    }
}