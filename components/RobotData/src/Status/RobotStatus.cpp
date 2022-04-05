#include "RobotStatus.h"

RobotStatus::RobotStatus(CarState initialState, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    robotState = new DataAbstract<CarState>(name, initialState);
    robotMap = new DataAbstract<bool>(name, false);
}