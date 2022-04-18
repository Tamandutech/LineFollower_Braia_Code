#include "CarStatusService.hpp"

QueueHandle_t CarStatusService::gpio_evt_queue;

void IRAM_ATTR CarStatusService::gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

CarStatusService::CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->latMarks = robot->getSLatMarks();
    this->PidTrans = robot->getPIDVel();

    mappingService = MappingService::getInstance();

    latMarks->marks->loadData();

    if (latMarks->marks->getSize() <= 0)
    {
        status->robotIsMapping->setData(true);
    }
    else
    {
        status->robotIsMapping->setData(false);
        mediaEncFinal = latMarks->marks->getData(latMarks->marks->getSize() - 1).MapEncMedia;
    }

    status->robotState->setData(CAR_STOPPED);

    stateChanged = true;
    lastState = status->robotState->getData();

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = GPIO_NUM_0;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
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

    uint32_t io_num;
    do
    {
        xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY);
    } while (io_num != GPIO_NUM_0);

    vTaskDelay(2500 / portTICK_PERIOD_MS);

    if (status->robotIsMapping->getData())
    {
        // Começa mapeamento
        mappingService->startNewMapping();
    }

    status->robotState->setData(CAR_IN_LINE);

    // Loop
    for (;;)
    {
        if (lastMappingState != status->robotIsMapping->getData())
        {
            lastMappingState = status->robotIsMapping->getData();

            speed->setToMapping();
        }
        else if (lastState != status->robotState->getData())
        {
            lastState = status->robotState->getData();

            if (lastState == CAR_IN_LINE)
                speed->setToLine();
            else
                speed->setToCurve();
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

        if (mediaEncActual >= mediaEncFinal && !status->robotIsMapping->getData() && actualCarState != CAR_STOPPED)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);

            // TODO: Encontrar forma bonita de suspender os outros serviços.
            // vTaskSuspend(xTaskPID);
            // vTaskSuspend(xTaskSensors);

            robot->getStatus()->robotState->setData(CAR_STOPPED);
        }

        if (!status->robotIsMapping->getData() && mediaEncActual < mediaEncFinal && actualCarState != CAR_STOPPED)
        {
            // define o status do carrinho se o mapeamento não estiver ocorrendo
            int mark = 0;

            for (mark = 0; mark < Marks; mark++)
            {
                // Verifica a conGetName().c_str()em do encoder e atribui o estado ao robô
                if (mark < Marks - 1)
                {
                    int32_t Manualmedia = latMarks->marks->getData(mark).MapEncMedia;        // Média dos encoders na chave mark
                    int32_t ManualmediaNxt = latMarks->marks->getData(mark + 1).MapEncMedia; // Média dos encoders na chave mark + 1

                    if (mediaEncActual >= Manualmedia && mediaEncActual <= ManualmediaNxt)
                    {                                                                 // análise do valor das médias dos encoders
                        int32_t mapstatus = latMarks->marks->getData(mark).MapStatus; // status do robô
                        CarState estado;

                        if (mapstatus == CAR_IN_LINE)
                        {
                            estado = CAR_IN_LINE;
                        }
                        else
                        {
                            estado = CAR_IN_CURVE;
                        }

                        status->robotState->setData(estado);
                        break;
                    }
                }
                else
                {
                    int32_t mapstatus = latMarks->marks->getData(mark).MapStatus; // status do robô
                    CarState estado;
                    if (mapstatus == CAR_IN_LINE)
                    {
                        estado = CAR_IN_LINE;
                    }
                    else
                    {
                        estado = CAR_IN_CURVE;
                    }
                    status->robotState->setData(estado);
                    break;
                }
            }
        }

        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}