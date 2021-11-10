#include "RobotStatus.h"

RobotStatus::RobotStatus(CarState initialState, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    this->robotState = initialState;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());
    
    ESP_LOGD(tag, "Criando Semáforos: %s", name.c_str());
    vSemaphoreCreateBinary(xSemaphoreRobotState);

}

int RobotStatus::setState(CarState value)
{
    if (xSemaphoreTake(xSemaphoreRobotState, (TickType_t)10) == pdTRUE)
    {
        this->robotState = value;
        xSemaphoreGive(xSemaphoreRobotState);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável robotState ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
CarState RobotStatus::getState()
{
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreRobotState, (TickType_t)10) == pdTRUE)
        {
            xSemaphoreGive(xSemaphoreRobotState);
            return this->robotState;
        }
        else
        {
            ESP_LOGE(tag, "Variável robotState ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}