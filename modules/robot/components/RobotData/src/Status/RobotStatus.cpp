#include "RobotStatus.h"

std::mutex RobotStatus::stateMutex;

RobotStatus::RobotStatus(CarState initialState, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);
    dataManager = dataManager->getInstance();
    robotState = new DataAbstract<uint8_t>("robotState", name, initialState);
    robotIsMapping = new DataAbstract<bool>("robotIsMapping", name, false);
    encreading = new DataAbstract<bool>("encreading", name, false);
    robotPaused = new DataAbstract<bool>("robotPaused", name, false);
    TrackStatus = new DataAbstract<uint8_t>("TrackStatus", name, 0);
    RealTrackStatus = new DataAbstract<uint8_t>("RealTrackStatus", name, 0);
    FirstMark = new DataAbstract<bool>("FirstMark", name, false);
    Transition = new DataAbstract<bool>("Transition", name, false);
    TunningMode = new DataAbstract<bool>("TunningMode", name, false);
    dataManager->registerParamData(TunningMode);
    HardDeleteMap = new DataAbstract<bool>("HardDeleteMap", name, false);
    dataManager->registerParamData(HardDeleteMap);
    LineColorBlack = new DataAbstract<bool>("LineColorBlack", name, false);
    dataManager->registerParamData(LineColorBlack);
    TuningMapped = new DataAbstract<bool>("TuningMapped", name, false);
    dataManager->registerParamData(TuningMapped);





}