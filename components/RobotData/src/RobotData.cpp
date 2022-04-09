#include "RobotData.h"

Robot::Robot(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = tag;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    storage = storage->getInstance();

    storage->mount_storage("/robotdata");

    storage->list_files();

    // Instânciando objetos componentes do Robô.
    ESP_LOGD(tag, "Criando sub-objetos para o %s", "Robô");

    this->sLatMarks = new dataSLatMarks("sLatMarks");
    ESP_LOGD(tag, "sLatMarks (%p)", this->sLatMarks);

    this->PIDVel = new dataPID("PIDVel");
    ESP_LOGD(tag, "PIDVel (%p)", this->PIDVel);

    this->PIDRot = new dataPID("PIDRot");
    ESP_LOGD(tag, "PIDRot (%p)", this->PIDRot);

    this->speed = new dataSpeed("speed");
    ESP_LOGD(tag, "speed (%p)", this->speed);

    this->sLat = new dataSensor(2, "sLat");
    ESP_LOGD(tag, "sLat (%p)", this->sLat);

    this->sArray = new dataSensor(8, "sArray");
    ESP_LOGD(tag, "sArray (%p)", this->sArray);

    this->Status = new RobotStatus(CAR_IN_LINE, "RobotStatus");
    ESP_LOGD(tag, "Status (%p)", this->Status);
    vSemaphoreCreateBinary(xSemaphorepacketstosend);
    vSemaphoreCreateBinary(xSemaphoreCarparam);

    // Inicializando os parâmetros do robô
    struct CarParameters initialParams;
    Setparams(initialParams);

    dataManager = dataManager->getInstance();
    dataManager->getRegistredParamDataCount();
    dataManager->loadAllParamData();
}

dataSLatMarks *Robot::getSLatMarks()
{
    return this->sLatMarks;
}

dataSpeed *Robot::getSpeed()
{
    return this->speed;
}

dataSensor *Robot::getsLat()
{
    return this->sLat;
}

dataSensor *Robot::getsArray()
{
    return this->sArray;
}

dataPID *Robot::getPIDVel()
{
    return this->PIDVel;
}

dataPID *Robot::getPIDRot()
{
    return this->PIDRot;
}

RobotStatus *Robot::getStatus()
{
    return this->Status;
}
bool Robot::PacketSendavailable()
{
    bool tempvar = false;
    if (xSemaphoreTake(xSemaphorepacketstosend, (TickType_t)10) == pdTRUE)
    {
        tempvar = !PacketstoSend.empty();
        xSemaphoreGive(xSemaphorepacketstosend);
        return tempvar;
    }
    else
    {
        ESP_LOGE(tag, "Variável PacketstoSend ocupada, não foi possível definir valor.");
        return tempvar;
    }
}

struct PacketData Robot::getPacketSend()
{
    struct PacketData tempvar;
    if (xSemaphoreTake(xSemaphorepacketstosend, (TickType_t)10) == pdTRUE)
    {
        tempvar = PacketstoSend.front();
        PacketstoSend.pop();
        xSemaphoreGive(xSemaphorepacketstosend);
        return tempvar;
    }
    else
    {
        ESP_LOGE(tag, "Variável PacketstoSend ocupada, não foi possível definir valor.");
        return tempvar;
    }
}

int Robot::addPacketSend(struct PacketData packet)
{
    if (xSemaphoreTake(xSemaphorepacketstosend, (TickType_t)10) == pdTRUE)
    {
        PacketstoSend.push(packet);
        xSemaphoreGive(xSemaphorepacketstosend);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável PacketstoSend ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int Robot::Setparams(struct CarParameters params)
{
    if (xSemaphoreTake(xSemaphoreCarparam, (TickType_t)10) == pdTRUE)
    {
        Carparam = params;
        xSemaphoreGive(xSemaphoreCarparam);
        Updateparams(params);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Carparam ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int Robot::Setparams()
{
    struct CarParameters tempParams;
    if (xSemaphoreTake(xSemaphoreCarparam, (TickType_t)10) == pdTRUE)
    {
        tempParams = Carparam;
        xSemaphoreGive(xSemaphoreCarparam);
        Updateparams(tempParams);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Carparam ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
struct CarParameters Robot::GetParams()
{
    struct CarParameters tempvar;
    if (xSemaphoreTake(xSemaphoreCarparam, (TickType_t)10) == pdTRUE)
    {
        tempvar = Carparam;
        xSemaphoreGive(xSemaphoreCarparam);
        return tempvar;
    }
    else
    {
        ESP_LOGE(tag, "Variável Carparam ocupada, não foi possível definir valor.");
        return tempvar;
    }
}

int Robot::Updateparams(struct CarParameters params)
{
    bool mappingState = getStatus()->robotMap->getData();
    auto PidRotParams = getPIDRot();
    auto PidVelParams = getPIDVel();
    auto speedParams = getSpeed();
    if (mappingState)
    {
        PidRotParams->Kp(CAR_IN_LINE)->setData(params.KpRotMapLine);
        PidRotParams->Kp(CAR_IN_CURVE)->setData(params.KpRotMapCurve);
        PidRotParams->Kd(CAR_IN_LINE)->setData(params.KdRotMapLine);
        PidRotParams->Kd(CAR_IN_CURVE)->setData(params.KdRotMapCurve);
        PidVelParams->Kp(CAR_IN_LINE)->setData(params.KpVelMapLine);
        PidVelParams->Kp(CAR_IN_CURVE)->setData(params.KpVelMapCurve);
        PidVelParams->Kd(CAR_IN_LINE)->setData(params.KdVelMapLine);
        PidVelParams->Kd(CAR_IN_CURVE)->setData(params.KdVelMapCurve);
        speedParams->SpeedMin(CAR_IN_LINE)->setData(params.SpeedMinMapLine);
        speedParams->SpeedMin(CAR_IN_CURVE)->setData(params.SpeedMinMapCurve);
        speedParams->SpeedMax(CAR_IN_LINE)->setData(params.SpeedMaxMapLine);
        speedParams->SpeedMax(CAR_IN_CURVE)->setData(params.SpeedMaxMapCurve);
        speedParams->SpeedBase(CAR_IN_LINE)->setData(params.SpeedBaseMapLine);
        speedParams->SpeedBase(CAR_IN_CURVE)->setData(params.SpeedBaseMapCurve);
    }
    else
    {
        PidRotParams->Kp(CAR_IN_LINE)->setData(params.KpRotRunLine);
        PidRotParams->Kp(CAR_IN_CURVE)->setData(params.KpRotRunCurve);
        PidRotParams->Kd(CAR_IN_LINE)->setData(params.KdRotRunLine);
        PidRotParams->Kd(CAR_IN_CURVE)->setData(params.KdRotRunCurve);
        PidVelParams->Kp(CAR_IN_LINE)->setData(params.KpVelRunLine);
        PidVelParams->Kp(CAR_IN_CURVE)->setData(params.KpVelRunCurve);
        PidVelParams->Kd(CAR_IN_LINE)->setData(params.KdVelRunLine);
        PidVelParams->Kd(CAR_IN_CURVE)->setData(params.KdVelRunCurve);
        speedParams->SpeedMin(CAR_IN_LINE)->setData(params.SpeedMinRunLine);
        speedParams->SpeedMin(CAR_IN_CURVE)->setData(params.SpeedMinRunCurve);
        speedParams->SpeedMax(CAR_IN_LINE)->setData(params.SpeedMaxRunLine);
        speedParams->SpeedMax(CAR_IN_CURVE)->setData(params.SpeedMaxRunCurve);
        speedParams->SpeedBase(CAR_IN_LINE)->setData(params.SpeedBaseRunLine);
        speedParams->SpeedBase(CAR_IN_CURVE)->setData(params.SpeedBaseRunCurve);
    }
    return RETORNO_OK;
}