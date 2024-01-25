#include "RobotData.h"

std::atomic<Robot *> Robot::instance;
std::mutex Robot::instanceMutex;

Robot::Robot(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;

    ESP_LOGD(name.c_str(), "Iniciando o ADC 1");
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    this->_adcHandle = adc_handle;

    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    storage = storage->getInstance();

    storage->mount_storage("/robotdata");

    storage->list_files();

    // Instânciando objetos componentes do Robô.
    ESP_LOGD(name.c_str(), "Criando sub-objetos para o %s", "Robô");

    this->Status = new RobotStatus("RobotStatus");
    ESP_LOGD(name.c_str(), "Status (%p)", this->Status);
    
    this->PID = new dataPID("PID"); 
    ESP_LOGD(name.c_str(), "PID (%p)", this->PID);
    
    this->speed = new dataSpeed("speed");
    ESP_LOGD(name.c_str(), "speed (%p)", this->speed);

    this->MappingData = new dataMapping("Mapping");
    ESP_LOGD(name.c_str(), "MappingData (%p)", this->MappingData);
    
    this->sideSensors = new dataSensor(2, "sideSensors");
    ESP_LOGD(name.c_str(), "sideSensors (%p)", this->sideSensors);

    this->frontSensors = new dataSensor(8, "frontSensors");
    ESP_LOGD(name.c_str(), "frontSensors (%p)", this->frontSensors);


    dataManager = dataManager->getInstance();
    dataManager->getRegistredParamDataCount();
    dataManager->loadAllParamData();
}

dataMapping *Robot::getMappingData()
{
    return this->MappingData;
}

dataSpeed *Robot::getSpeed()
{
    return this->speed;
}

dataSensor *Robot::getsideSensors()
{
    return this->sideSensors;
}

dataSensor *Robot::getfrontSensors()
{
    return this->frontSensors;
}

dataPID *Robot::getPID()
{
    return this->PID;
}

RobotStatus *Robot::getStatus()
{
    return this->Status;
}


std::string Robot::GetName()
{
    return this->name;
}

adc_oneshot_unit_handle_t Robot::getADC_handle()
{
    return this->_adcHandle;
}