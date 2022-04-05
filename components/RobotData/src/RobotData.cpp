#include "RobotData.h"

Robot::Robot(std::string name)
{
    
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = tag;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

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
bool Robot::PacketSendavailable(){
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

struct PacketData Robot::getPacketSend(){
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

int Robot::addPacketSend(struct PacketData packet){
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
int Robot::Setparams(struct CarParameters params){
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
int Robot::Setparams(){
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
struct CarParameters Robot::GetParams(){
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

 int Robot::Updateparams(struct CarParameters params){
     bool mappingState = getStatus()->getMapping();
     auto PidRotParams = getPIDRot();
     auto PidVelParams = getPIDVel();
     auto speedParams = getSpeed();
     if(mappingState){
         PidRotParams->setKp(params.KpRotMapLine,CAR_IN_LINE);
         PidRotParams->setKp(params.KpRotMapCurve,CAR_IN_CURVE);
         PidRotParams->setKd(params.KdRotMapLine,CAR_IN_LINE);
         PidRotParams->setKd(params.KdRotMapCurve,CAR_IN_CURVE);
         PidVelParams->setKp(params.KpVelMapLine,CAR_IN_LINE);
         PidVelParams->setKp(params.KpVelMapCurve,CAR_IN_CURVE);
         PidVelParams->setKd(params.KdVelMapLine,CAR_IN_LINE);
         PidVelParams->setKd(params.KdVelMapCurve,CAR_IN_CURVE);
         speedParams->setSpeedMin(params.SpeedMinMapLine,CAR_IN_LINE);
         speedParams->setSpeedMin(params.SpeedMinMapCurve,CAR_IN_CURVE);
         speedParams->setSpeedMax(params.SpeedMaxMapLine,CAR_IN_LINE);
         speedParams->setSpeedMax(params.SpeedMaxMapCurve,CAR_IN_CURVE);
         speedParams->setSpeedBase(params.SpeedBaseMapLine,CAR_IN_LINE);
         speedParams->setSpeedBase(params.SpeedBaseMapCurve,CAR_IN_CURVE);
     }
     else{
         PidRotParams->setKp(params.KpRotRunLine,CAR_IN_LINE);
         PidRotParams->setKp(params.KpRotRunCurve,CAR_IN_CURVE);
         PidRotParams->setKd(params.KdRotRunLine,CAR_IN_LINE);
         PidRotParams->setKd(params.KdRotRunCurve,CAR_IN_CURVE);
         PidVelParams->setKp(params.KpVelRunLine,CAR_IN_LINE);
         PidVelParams->setKp(params.KpVelRunCurve,CAR_IN_CURVE);
         PidVelParams->setKd(params.KdVelRunLine,CAR_IN_LINE);
         PidVelParams->setKd(params.KdVelRunCurve,CAR_IN_CURVE);
         speedParams->setSpeedMin(params.SpeedMinRunLine,CAR_IN_LINE);
         speedParams->setSpeedMin(params.SpeedMinRunCurve,CAR_IN_CURVE);
         speedParams->setSpeedMax(params.SpeedMaxRunLine,CAR_IN_LINE);
         speedParams->setSpeedMax(params.SpeedMaxRunCurve,CAR_IN_CURVE);
         speedParams->setSpeedBase(params.SpeedBaseRunLine,CAR_IN_LINE);
         speedParams->setSpeedBase(params.SpeedBaseRunCurve,CAR_IN_CURVE);
     }
     return RETORNO_OK;
 }