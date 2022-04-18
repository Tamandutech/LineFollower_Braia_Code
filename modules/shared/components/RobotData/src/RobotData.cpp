#include "RobotData.h"

std::atomic<Robot *> Robot::instance;
std::mutex Robot::instanceMutex;

Robot::Robot(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    storage = storage->getInstance();

    storage->mount_storage("/robotdata");

    storage->list_files();

    // Instânciando objetos componentes do Robô.
    ESP_LOGD(name.c_str(), "Criando sub-objetos para o %s", "Robô");

    this->sLatMarks = new dataSLatMarks("sLatMarks");
    ESP_LOGD(name.c_str(), "sLatMarks (%p)", this->sLatMarks);

    this->PIDVel = new dataPID("PIDVel");
    ESP_LOGD(name.c_str(), "PIDVel (%p)", this->PIDVel);

    this->PIDRot = new dataPID("PIDRot");
    ESP_LOGD(name.c_str(), "PIDRot (%p)", this->PIDRot);

    this->speed = new dataSpeed("speed");
    ESP_LOGD(name.c_str(), "speed (%p)", this->speed);

    this->sLat = new dataSensor(2, "sLat");
    ESP_LOGD(name.c_str(), "sLat (%p)", this->sLat);

    this->sArray = new dataSensor(8, "sArray");
    ESP_LOGD(name.c_str(), "sArray (%p)", this->sArray);

    this->Status = new RobotStatus(CAR_IN_LINE, "RobotStatus");
    ESP_LOGD(name.c_str(), "Status (%p)", this->Status);

    // Inicializando os parâmetros do robô
    // struct CarParameters initialParams;
    // Setparams(initialParams);

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