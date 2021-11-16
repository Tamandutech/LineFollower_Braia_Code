#ifndef DATA_SPEED_H
#define DATA_SPEED_H

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "dataEnums.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

class dataSpeed
{
public:
    dataSpeed(std::string name = "dataSpeed");

    // Velocidades Atuais
    DataFunction setRPMRight_inst(int16_t value);
    int16_t getRPMRight_inst();

    DataFunction setRPMLeft_inst(int16_t value);
    int16_t getRPMLeft_inst();

    DataFunction setRPMCar_media(int16_t value);
    int16_t getRPMCar_media();

    // Valores de Parametros
    DataFunction setMPR_MotEsq(uint16_t Revolucao, uint16_t Reducao);
    uint16_t getMPR_MotEsq();

    DataFunction setMPR_MotDir(uint16_t Revolucao, uint16_t Reducao);
    uint16_t getMPR_MotDir();

    // Valores variaveis
    DataFunction setSpeedLeft(int8_t value, CarState carState);
    int8_t getSpeedLeft(CarState carState);

    DataFunction setSpeedRight(int8_t value, CarState carState);
    int8_t getSpeedRight(CarState carState);

    DataFunction setSpeedMax(int8_t value, CarState carState);
    int8_t getSpeedMax(CarState carState);

    DataFunction setSpeedMin(int8_t value, CarState carState);
    int8_t getSpeedMin(CarState carState);

    DataFunction setSpeedBase(int8_t value, CarState carState);
    int8_t getSpeedBase(CarState carState);

private:
    std::string name;
    const char *tag = "RobotData";

    template <class setVarType>
    DataFunction setVar(setVarType value, setVarType *var, SemaphoreHandle_t *xSemaphoreOfVar)
    {
        if (xSemaphoreTake((*xSemaphoreOfVar), (TickType_t)10) == pdTRUE)
        {
            (*var) = value;
            xSemaphoreGive((*xSemaphoreOfVar));
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
    }

    template <class getVarType>
    getVarType getVar(getVarType *var, SemaphoreHandle_t *xSemaphoreOfVar)
    {
        uint16_t tempChannel;
        for (;;)
        {
            if (xSemaphoreTake((*xSemaphoreOfVar), (TickType_t)10) == pdTRUE)
            {
                tempChannel = (*var);
                xSemaphoreGive((*xSemaphoreOfVar));
                return tempChannel;
            }
            else
            {
                ESP_LOGE(tag, "Variável ocupada, não foi possível ler valor. Tentando novamente...");
            }
        }
    }

    // Parametros

    /*
     * Variavel que contempla relacao de Revloucoes e reducao
     * dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
     * */
    uint16_t MPR_MotEsq;
    SemaphoreHandle_t xSemaphoreMPR_MotEsq;
    uint16_t MPR_MotDir;
    SemaphoreHandle_t xSemaphoreMPR_MotDir;

    // Valocidades atuais
    SemaphoreHandle_t xSemaphorerevsRight_inst;
    int16_t revsRight_inst;
    SemaphoreHandle_t xSemaphorerevsLeft_inst;
    int16_t revsLeft_inst;
    SemaphoreHandle_t xSemaphorerevsCar_media;
    int16_t revsCar_media;

    // Linha
    SemaphoreHandle_t xSemaphoreright_line;
    int8_t right_line;
    SemaphoreHandle_t xSemaphoreleft_line;
    int8_t left_line;
    SemaphoreHandle_t xSemaphoremax_line;
    int8_t max_line;
    SemaphoreHandle_t xSemaphoremin_line;
    int8_t min_line;
    SemaphoreHandle_t xSemaphorebase_line;
    int8_t base_line;

    // Curva
    SemaphoreHandle_t xSemaphoreright_curve;
    int8_t right_curve;
    SemaphoreHandle_t xSemaphoreleft_curve;
    int8_t left_curve;
    SemaphoreHandle_t xSemaphoremax_curve;
    int8_t max_curve;
    SemaphoreHandle_t xSemaphoremin_curve;
    int8_t min_curve;
    SemaphoreHandle_t xSemaphorebase_curve;
    int8_t base_curve;
};

#endif