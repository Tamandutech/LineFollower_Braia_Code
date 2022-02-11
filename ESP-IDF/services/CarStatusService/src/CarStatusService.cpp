#include "CarStatusService.hpp"

CarStatusService::CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->latMarks = robot->getSLatMarks();
    this->PidTrans = robot->getPIDVel();

    latMarks->SetTotalLeftMarks(3);
    latMarks->SetFinalMark(5472);
    Marks = latMarks->getTotalLeftMarks() + 1; // marcas laterais esquerda na pista

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

    status->setMapping(false);
    status->setState(CAR_STOPPED);
    latMarks->SetMapFinished(false);

    mapChanged = true;
    lastmapstate = status->getMapping();
}

void CarStatusService::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();
    int32_t PlusPulses = 0;                       // Pulsos a mais para a parada
    int iloop = 0;
    // Loop
    for (;;)
    {
        CarState parar = status->getState(); // Verifica se o carro deve se manter parado
        bool bottom = gpio_get_level(GPIO_NUM_0);
        if(!bottom && !status->getMapping()){
            status->setState(CAR_IN_LINE);
            status->setMapping(true);
            latMarks->SetMapFinished(false);
            latMarks->SetrightMarks(0);
        }
        if(lastmapstate != status->getMapping()){
            lastmapstate = status->getMapping();
            mapChanged = true;
        }
        int32_t FinalMark = latMarks->getFinalMark(); // Media dos encoders da marcação final
        Marks = latMarks->getTotalLeftMarks() + 1;
        int32_t mediaEnc = (speed->getEncRight() + speed->getEncLeft()) / 2; // calcula media dos encoders

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
        if (iloop >= 20 && !status->getMapping())
        {
            ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->getState());
            ESP_LOGD(GetName().c_str(), "EncMedia: %d", mediaEnc);
            ESP_LOGD(GetName().c_str(), "FinalMark: %d", FinalMark);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PidTrans->getSetpoint());
            iloop = 0;
        }
        iloop++;
#endif

        if (mediaEnc >= FinalMark + PlusPulses && !status->getMapping() && parar != CAR_STOPPED)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);

            // TODO: Encontrar forma bonita de suspender os outros serviços.
            // vTaskSuspend(xTaskPID);
            // vTaskSuspend(xTaskSensors);

            robot->getStatus()->setState(CAR_STOPPED);
        }
        if (!status->getMapping() && mediaEnc < FinalMark + PlusPulses && parar != CAR_STOPPED)
        { // define o status do carrinho se o mapeamento não estiver ocorrendo
            int mark = 0;
            for (mark = 0; mark < Marks; mark++)
            { // Verifica a conGetName().c_str()em do encoder e atribui o estado ao robô
                if (mark < Marks - 1)
                {
                    int32_t Manualmedia = (latMarks->getMarkDataReg(mark)).MapEncMedia; // Média dos encoders na chave mark    
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
                        status->setState(estado);
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
                    status->setState(estado);
                    break;
                }
            }
        }
        if (robot->getStatus()->getMapping() && mapChanged)
        {
            mapChanged = false;

            robot->getSpeed()->setSpeedBase(25, CAR_IN_LINE);
            robot->getSpeed()->setSpeedBase(25, CAR_IN_CURVE);

            robot->getSpeed()->setSpeedMax(50, CAR_IN_LINE);
            robot->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

            robot->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
            robot->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

            robot->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
            robot->getPIDVel()->setKd(0.000, CAR_IN_LINE);
            robot->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
            robot->getPIDVel()->setKd(0.000, CAR_IN_CURVE);

            robot->getPIDRot()->setKi(0.00, CAR_IN_LINE);
            robot->getPIDVel()->setKi(0.00, CAR_IN_LINE);
            robot->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
            robot->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

            robot->getPIDRot()->setKp(0.27, CAR_IN_LINE);
            robot->getPIDVel()->setKp(0.035, CAR_IN_LINE);
            robot->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
            robot->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

        }
        else if(!robot->getStatus()->getMapping() && mapChanged)
        {
            mapChanged = false;

            robot->getSpeed()->setSpeedBase(40, CAR_IN_LINE);
            robot->getSpeed()->setSpeedBase(20, CAR_IN_CURVE);

            robot->getSpeed()->setSpeedMax(70, CAR_IN_LINE);
            robot->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

            robot->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
            robot->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

            robot->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
            robot->getPIDVel()->setKd(0.0, CAR_IN_LINE);
            robot->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
            robot->getPIDVel()->setKd(0.0, CAR_IN_CURVE);

            robot->getPIDRot()->setKi(0.00, CAR_IN_LINE);
            robot->getPIDVel()->setKi(0.00, CAR_IN_LINE);
            robot->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
            robot->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

            robot->getPIDRot()->setKp(0.27, CAR_IN_LINE);
            robot->getPIDVel()->setKp(0.035, CAR_IN_LINE);
            robot->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
            robot->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

        }
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
    }
}