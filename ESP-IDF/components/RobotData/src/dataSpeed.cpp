#include "dataSpeed.h"

dataSpeed::dataSpeed(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());

    ESP_LOGD(tag, "Criando Semáforos: %s", name.c_str());
    vSemaphoreCreateBinary(xSemaphoreMPR_MotEsq);
    vSemaphoreCreateBinary(xSemaphoreMPR_MotDir);
    vSemaphoreCreateBinary(xSemaphorerevsRight_inst);
    vSemaphoreCreateBinary(xSemaphorerevsLeft_inst);
    vSemaphoreCreateBinary(xSemaphorerevsCar_media);
    vSemaphoreCreateBinary(xSemaphoreright_line);
    vSemaphoreCreateBinary(xSemaphoreleft_line);
    vSemaphoreCreateBinary(xSemaphoremax_line);
    vSemaphoreCreateBinary(xSemaphoremin_line);
    vSemaphoreCreateBinary(xSemaphorebase_line);
    vSemaphoreCreateBinary(xSemaphoreright_curve);
    vSemaphoreCreateBinary(xSemaphoreleft_curve);
    vSemaphoreCreateBinary(xSemaphoremax_curve);
    vSemaphoreCreateBinary(xSemaphoremin_curve);
    vSemaphoreCreateBinary(xSemaphorebase_curve);
}

// Metodos de valores variaveis

DataFunction dataSpeed::setSpeedLeft(int8_t value, CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return setVar(value, &this->left_line, &xSemaphoreleft_line);
        break;

    case CAR_IN_CURVE:
        return setVar(value, &this->left_curve, &xSemaphoreleft_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedLeft não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
int8_t dataSpeed::getSpeedLeft(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return getVar(&this->left_line, &xSemaphoreleft_line);
        break;

    case CAR_IN_CURVE:
        return getVar(&this->left_curve, &xSemaphoreleft_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedLeft não será lido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}

DataFunction dataSpeed::setSpeedRight(int8_t value, CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return setVar(value, &this->right_line, &xSemaphoreright_line);
        break;

    case CAR_IN_CURVE:
        return setVar(value, &this->right_curve, &xSemaphoreright_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedRight não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
int8_t dataSpeed::getSpeedRight(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return getVar(&this->right_line, &xSemaphoreright_line);
        break;

    case CAR_IN_CURVE:
        return getVar(&this->right_curve, &xSemaphoreright_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedLeft não será lido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}

DataFunction dataSpeed::setSpeedMax(int8_t value, CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return setVar(value, &this->max_line, &xSemaphoremax_line);
        break;

    case CAR_IN_CURVE:
        return setVar(value, &this->max_curve, &xSemaphoremax_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaMax não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
int8_t dataSpeed::getSpeedMax(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return getVar(&this->max_line, &xSemaphoremax_line);
        break;

    case CAR_IN_CURVE:
        return getVar(&this->max_curve, &xSemaphoremax_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaMax não será lido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}

DataFunction dataSpeed::setSpeedMin(int8_t value, CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return setVar(value, &this->min_line, &xSemaphoremin_line);
        break;

    case CAR_IN_CURVE:
        return setVar(value, &this->min_curve, &xSemaphoremin_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaMin não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
int8_t dataSpeed::getSpeedMin(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return getVar(&this->min_line, &xSemaphoremin_line);
        break;

    case CAR_IN_CURVE:
        return getVar(&this->min_curve, &xSemaphoremin_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaMin não será lido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}

DataFunction dataSpeed::setSpeedBase(int8_t value, CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return setVar(value, &this->base_line, &xSemaphorebase_line);
        break;

    case CAR_IN_CURVE:
        return setVar(value, &this->base_curve, &xSemaphorebase_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaBase não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
int8_t dataSpeed::getSpeedBase(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_LINE:
        return getVar(&this->base_line, &xSemaphorebase_line);
        break;

    case CAR_IN_CURVE:
        return getVar(&this->base_curve, &xSemaphorebase_curve);
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de SpeedMediaBase não será lido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}

// Metodos de valores de parametros
DataFunction dataSpeed::setMPR_MotEsq(uint16_t Revolucao, uint16_t Reducao)
{
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMPR_MotEsq, (TickType_t)10) == pdTRUE)
        {
            // Calculo
            this->MPR_MotEsq = Revolucao * Reducao;
            xSemaphoreGive(xSemaphoreMPR_MotEsq);
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
uint16_t dataSpeed::getMPR_MotEsq()
{
    uint16_t tempVar = 0;

    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMPR_MotEsq, (TickType_t)10) == pdTRUE)
        {
            tempVar = this->MPR_MotEsq;
            xSemaphoreGive(xSemaphoreMPR_MotEsq);
            return tempVar;
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

DataFunction dataSpeed::setMPR_MotDir(uint16_t Revolucao, uint16_t Reducao)
{
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMPR_MotDir, (TickType_t)10) == pdTRUE)
        {
            this->MPR_MotDir = Revolucao * Reducao;
            xSemaphoreGive(xSemaphoreMPR_MotDir);
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
uint16_t dataSpeed::getMPR_MotDir()
{
    uint16_t tempVar = 0;

    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMPR_MotDir, (TickType_t)10) == pdTRUE)
        {
            tempVar = this->MPR_MotDir;
            xSemaphoreGive(xSemaphoreMPR_MotDir);
            return tempVar;
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

// Metodos de valores de velocidades Atuais
DataFunction dataSpeed::setRPMRight_inst(int16_t value){
    return setVar(value, &revsRight_inst, &xSemaphorerevsRight_inst);
}
int16_t dataSpeed::getRPMRight_inst(){
    return getVar(&revsRight_inst, &xSemaphorerevsRight_inst);
}

DataFunction dataSpeed::setRPMLeft_inst(int16_t value){
    return setVar(value, &revsLeft_inst, &xSemaphorerevsLeft_inst);
}
int16_t dataSpeed::getRPMLeft_inst(){
    return getVar(&revsLeft_inst, &xSemaphorerevsLeft_inst);
}

DataFunction dataSpeed::setRPMCar_media(int16_t value){
    return setVar(value, &revsCar_media, &xSemaphorerevsCar_media);
}
int16_t dataSpeed::getRPMCar_media(){
    return getVar(&revsCar_media, &xSemaphorerevsCar_media);
}