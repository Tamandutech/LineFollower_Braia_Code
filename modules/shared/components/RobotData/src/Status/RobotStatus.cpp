#include "RobotStatus.h"

std::mutex RobotStatus::stateMutex;

RobotStatus::RobotStatus(CarState initialState, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    robotState = new DataAbstract<uint8_t>("robotState", name, initialState);
    robotIsMapping = new DataAbstract<bool>("robotIsMapping", name, false);
    encreading = new DataAbstract<bool>("encreading", name, false);
    ColorLed0 = new DataAbstract<uint32_t>("ColorLed0", name, 0);
    ColorLed1 = new DataAbstract<uint32_t>("ColorLed0", name, 0);
    ColorLed2 = new DataAbstract<uint32_t>("ColorLed0", name, 0);
}