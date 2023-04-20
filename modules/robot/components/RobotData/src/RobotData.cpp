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

    this->Status = new RobotStatus(CAR_IN_LINE, "RobotStatus");
    ESP_LOGD(name.c_str(), "Status (%p)", this->Status);
    this->Status->PID_Select->loadData();
    
    
    // Seleciona o PID a ser utilizado
    if(this->Status->PID_Select->getData())
    { 
        this->PIDClassic = new dataPID("PIDClassic"); 
        ESP_LOGD(name.c_str(), "PIDClassic (%p)", this->PIDClassic);
    }
    else
    {

        this->PIDVel = new dataPID("PIDVel");
        ESP_LOGD(name.c_str(), "PIDVel (%p)", this->PIDVel);

        this->PIDRot = new dataPID("PIDRot");
        ESP_LOGD(name.c_str(), "PIDRot (%p)", this->PIDRot);

        this->PIDIR = new dataPID("PIDIR");
        ESP_LOGD(name.c_str(), "PIDIR (%p)", this->PIDIR);

    }

    
    this->speed = new dataSpeed("speed", this->Status->PID_Select->getData());
    ESP_LOGD(name.c_str(), "speed (%p)", this->speed);

    this->sLatMarks = new dataSLatMarks("sLatMarks");
    ESP_LOGD(name.c_str(), "sLatMarks (%p)", this->sLatMarks);
    
    this->sLat = new dataSensor(2, "sLat");
    ESP_LOGD(name.c_str(), "sLat (%p)", this->sLat);

    this->sArray = new dataSensor(8, "sArray");
    ESP_LOGD(name.c_str(), "sArray (%p)", this->sArray);



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

dataPID *Robot::getPIDClassic()
{
    return this->PIDClassic;
}

dataPID *Robot::getPIDIR()
{
    return this->PIDIR;
}

dataPID *Robot::getPIDRot()
{
    return this->PIDRot;
}

RobotStatus *Robot::getStatus()
{
    return this->Status;
}


std::string Robot::GetName()
{
    return this->name;
}