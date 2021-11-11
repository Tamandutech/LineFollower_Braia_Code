#include "RobotData.h"

Robot::Robot(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = tag;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());

    // Instânciando objetos componentes do Robô.
    ESP_LOGD(tag, "Criando sub-objetos para o %s", "Robô");


    this->sLatMarks = new dataSLatMarks("sLatMarks");
    this->PIDVel = new dataPID("PIDVel");
    this->PIDRot = new dataPID("PIDRot");
    this->speed = new dataSpeed("speed");
    this->sLat = new dataSensor(2, "sLat");
    this->sArray = new dataSensor(8, "sArray");
    this->Status = new RobotStatus(CAR_IN_LINE, "RobotStatus");
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

RobotStatus *Robot::getStatus(){
    return this->Status;
}