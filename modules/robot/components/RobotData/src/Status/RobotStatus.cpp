#include "RobotStatus.h"


RobotStatus::RobotStatus(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);
    dataManager = dataManager->getInstance();
    robotState = new DataAbstract<uint8_t>("robotState", name);
    transitionTrackSegment = new DataAbstract<uint8_t>("transitionTrackSegment", name, 0);
    currentTrackSegment = new DataAbstract<uint8_t>("currentTrackSegment", name, 0);
    TunningMode = new DataAbstract<bool>("TunningMode", name, false);
    dataManager->registerParamData(TunningMode);
    OpenLoopControl = new DataAbstract<bool>("OpenLoopControl", name, false);
    dataManager->registerParamData(OpenLoopControl);
    HardDeleteMap = new DataAbstract<bool>("HardDeleteMap", name, false);
    dataManager->registerParamData(HardDeleteMap);
    LineColorBlack = new DataAbstract<bool>("LineColorBlack", name, false);
    dataManager->registerParamData(LineColorBlack);
    OpenLoopTreshold = new DataAbstract<uint16_t>("OpenLoopTreshold", name, 3000);
    dataManager->registerParamData(OpenLoopTreshold);
}
