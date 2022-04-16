#include "CarStatusService.hpp"

CarStatusService::CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->latMarks = robot->getSLatMarks();
    this->PidTrans = robot->getPIDVel();

    latMarks->totalLeftMarks->setData(3);
    latMarks->finalMark->setData(5472);
    Marks = latMarks->totalLeftMarks->getData() + 1; // marcas laterais esquerda na pista

#if defined(ManualMap)
    struct MapData marktest;
    marktest.MapEncMedia = 0;
    marktest.MapTime = 0;
    marktest.MapStatus = CAR_IN_LINE;
    latMarks->SetMarkDataReg(marktest, 0);
    marktest.MapEncMedia = 1100;
    marktest.MapTime = 990;
    marktest.MapStatus = CAR_IN_CURVE;
    latMarks->SetMarkDataReg(marktest, 1);
    marktest.MapEncMedia = 2554;
    marktest.MapTime = 3210;
    marktest.MapStatus = CAR_IN_LINE;
    latMarks->SetMarkDataReg(marktest, 2);
    marktest.MapEncMedia = 4999;
    marktest.MapTime = 7470;
    marktest.MapStatus = CAR_IN_CURVE;
    latMarks->SetMarkDataReg(marktest, 3);
#endif

    status->robotMap->setData(false);
    status->robotState->setData(CAR_STOPPED);
    latMarks->mapFinished->setData(false);

    mapChanged = true;
    lastmapstate = status->robotMap->getData();
}

void CarStatusService::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();
    int32_t PlusPulses = 0; // Pulsos a mais para a parada
    int iloop = 0;
    // Loop
    for (;;)
    {
        CarState parar = (CarState) status->robotState->getData(); // Verifica se o carro deve se manter parado
        bool bottom = gpio_get_level(GPIO_NUM_0);

        if (!bottom)
        {
            vTaskDelay(2500 / portTICK_PERIOD_MS);
            bottom = gpio_get_level(GPIO_NUM_0);
            if (bottom && !status->robotMap->getData())
            { // Começa mapeamento
                status->robotState->setData(CAR_IN_LINE);
                status->robotMap->setData(true);
                latMarks->mapFinished->setData(false);
                latMarks->rightMarks->setData(0);
                latMarks->leftMarks->setData(0);
            }
            else if (!bottom)
            { // Começa a usar dados do encoder para completar a pista
                status->robotState->setData(CAR_IN_LINE);
                status->robotMap->setData(false);
                latMarks->mapFinished->setData(true);
                latMarks->rightMarks->setData(0);
                latMarks->leftMarks->setData(0);
            }
        }
        if (lastmapstate != status->robotMap->getData())
        {
            lastmapstate = status->robotMap->getData();
            mapChanged = true;
        }

        int32_t FinalMark = latMarks->finalMark->getData(); // Media dos encoders da marcação final
        Marks = latMarks->totalLeftMarks->getData() + 1;
        int32_t mediaEnc = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2; // calcula media dos encoders

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
        if (iloop >= 20 && !status->robotMap->getData())
        {
            ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
            ESP_LOGD(GetName().c_str(), "EncMedia: %d", mediaEnc);
            ESP_LOGD(GetName().c_str(), "FinalMark: %d", FinalMark);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PidTrans->setpoint->getData());
            iloop = 0;
        }
        iloop++;
#endif

        if (mediaEnc >= FinalMark + PlusPulses && !status->robotMap->getData() && parar != CAR_STOPPED)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);

            // TODO: Encontrar forma bonita de suspender os outros serviços.
            // vTaskSuspend(xTaskPID);
            // vTaskSuspend(xTaskSensors);

            robot->getStatus()->robotState->setData(CAR_STOPPED);
        }
        if (!status->robotMap->getData() && mediaEnc < FinalMark + PlusPulses && parar != CAR_STOPPED)
        { // define o status do carrinho se o mapeamento não estiver ocorrendo
            int mark = 0;
            for (mark = 0; mark < Marks; mark++)
            { // Verifica a conGetName().c_str()em do encoder e atribui o estado ao robô
                if (mark < Marks - 1)
                {
                    int32_t Manualmedia = (latMarks->getMarkDataReg(mark)).MapEncMedia;        // Média dos encoders na chave mark
                    int32_t ManualmediaNxt = (latMarks->getMarkDataReg(mark + 1)).MapEncMedia; // Média dos encoders na chave mark + 1
                    if (mediaEnc >= Manualmedia && mediaEnc <= ManualmediaNxt)
                    {                                                                   // análise do valor das médias dos encoders
                        int32_t mapstatus = (latMarks->getMarkDataReg(mark)).MapStatus; // status do robô
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
                    int32_t mapstatus = (latMarks->getMarkDataReg(mark)).MapStatus; // status do robô
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
        if (mapChanged)
        {
            // mapChanged = false;
            // robot->Setparams(); // Atualiza os parâmetros do robô

            // robot->getPIDRot()->Kp(CAR_IN_LINE)->setData(0.27);
            // robot->getPIDVel()->Kp(CAR_IN_LINE)->setData(0.05);
            // robot->getPIDRot()->Kp(CAR_IN_CURVE)->setData(0.27);
            // robot->getPIDVel()->Kp(CAR_IN_CURVE)->setData(0.05);
        }
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}