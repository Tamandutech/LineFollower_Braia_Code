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