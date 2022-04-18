#ifndef DATA_SLATMARKS_H
#define DATA_SLATMARKS_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <cstring>

#include "dataEnums.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"
#include "DataMap.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class dataSLatMarks
{
public:
    dataSLatMarks(std::string name = "dataSensor");

    // Estado do sensor da lateral esquerda
    DataAbstract<bool> *latEsqPass;
    // Estado do sensor da lateral direita
    DataAbstract<bool> *latDirPass;

    // Quantidade atual de marcas da lateral esquerda
    DataAbstract<uint16_t> *leftMarks;
    // Quantidade atual de marcas da lateral direita
    DataAbstract<uint16_t> *rightMarks;

    // Média da contagem dos encoders para última marcação da pista
    DataAbstract<int32_t> *finalEncPulses;

    // Estrutura de dados que armazena os dados de mapeamento
    DataMap *marks;

    // Parâmetros

    // Limite de variação em milimetros de distância percorrida entre as rodas para considerar que o carro fez uma curva
    DataAbstract<uint8_t> *thresholdToCurve;

    // Incrementa a contagem de marcas da lateral esquerda
    void leftPassedInc();
    // Incrementa a contagem de marcas da lateral direita
    void rightPassedInc();

private:
    std::string name;

    DataManager *dataManager;
};

#endif