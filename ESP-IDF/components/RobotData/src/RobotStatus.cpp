#include "RobotStatus.h"

RobotStatus::RobotStatus(CarState initialState, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    this->robotState = initialState;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    ESP_LOGD(tag, "Criando Semáforos: %s", name.c_str());
    vSemaphoreCreateBinary(xSemaphoreRobotState);
    vSemaphoreCreateBinary(xSemaphoreRobotMap);
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
    CarState tempvar;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreRobotState, (TickType_t)10) == pdTRUE)
        {
            tempvar = this->robotState; 
            xSemaphoreGive(xSemaphoreRobotState);
            return tempvar;
        }
        else
        {
            ESP_LOGE(tag, "Variável robotState ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

int RobotStatus::setMapping(bool value)
{
    if (xSemaphoreTake(xSemaphoreRobotMap, (TickType_t)10) == pdTRUE)
    {
        this->robotMap = value;
        xSemaphoreGive(xSemaphoreRobotMap);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável robotMap ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
bool RobotStatus::getMapping()
{
    bool tempvar;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreRobotMap, (TickType_t)10) == pdTRUE)
        {
            tempvar = this->robotMap;
            xSemaphoreGive(xSemaphoreRobotMap);
            return tempvar;
        }
        else
        {
            ESP_LOGE(tag, "Variável robotMap ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}