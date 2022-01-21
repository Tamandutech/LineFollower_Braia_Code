#include "CarStatusService.hpp"

CarStatusService::CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
}

void CarStatusService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->getState());
        int32_t mediaEnc = (speed->getEncRight() + speed->getEncLeft()) / 2; // calcula media dos encoders

        if (mediaEnc >= FinalMark + PlusPulses)
            robot->getStatus()->setState(CAR_STOPPED);

        if (!status->getMapping() && robot->getSLatMarks()->getrightMarks() < 2 && mediaEnc < FinalMark + PlusPulses)
        {
            // define o status do carrinho se o mapeamento não estiver ocorrendo
            int mark = 0;
            for (mark = 0; mark < Marks; mark++)
            {
                // Verifica a contagem do encoder e atribui o estado ao robô
                if (mark < Marks - 1)
                {
                    int32_t Manualmedia = Manualmap[0][mark];
                    int32_t ManualmediaNxt = Manualmap[0][mark + 1];

                    if (mediaEnc >= Manualmedia && mediaEnc <= ManualmediaNxt)
                    {
                        status->setState(Manualmap[1][mark] == CAR_IN_LINE ? CAR_IN_LINE : CAR_IN_CURVE);
                        break;
                    }
                }
                else
                {
                    status->setState(Manualmap[1][mark] == CAR_IN_LINE ? CAR_IN_LINE : CAR_IN_CURVE);
                    break;
                }
            }
        }

        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}