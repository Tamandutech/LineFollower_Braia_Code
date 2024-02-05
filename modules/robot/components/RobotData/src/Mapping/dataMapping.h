#ifndef DATA_MAPPING_H
#define DATA_MAPPING_H

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

#include "esp_log.h"

class dataMapping
{
public:
    dataMapping(std::string name = "dataMapping");

    // Estado do sensor da lateral esquerda
    DataAbstract<bool> *latEsqPass;
    // Estado do sensor da lateral direita
    DataAbstract<bool> *latDirPass;

    // Quantidade atual de marcas da lateral esquerda
    DataAbstract<uint16_t> *leftMarks;
    // Quantidade atual de marcas da lateral direita
    DataAbstract<uint16_t> *rightMarks;

    // Estrutura de dados que armazena os dados de mapeamento
    DataMap *TrackSideMarks;

    // Parâmetros

    // Número de leituras dos sensores laterais para determinar a média que será utilizada na contagem de marcações laterais 
    DataAbstract<uint16_t> *MarkstoMean;
    //Número de marcações direita para a parada 
    DataAbstract<uint16_t> *MarkstoStop;
    //Pulsos antes de inicar uma curva para iniciar a desaceleração
    DataAbstract<uint32_t> *PulsesBeforeCurve;
    
    // Limite de variação em milimetros de distância percorrida entre as rodas para considerar que o carro fez uma curva
    DataAbstract<uint8_t> *thresholdToCurve;

    // Condições para determinar que tipo de cada trecho da pista em mm
    DataAbstract<uint16_t> *LongLineLength;
    DataAbstract<uint16_t> *MediumLineLength;
    DataAbstract<uint16_t> *LongCurveLength;
    DataAbstract<uint16_t> *MediumCurveLength;

    // Incrementa a contagem de marcas da lateral esquerda
    void leftPassedInc();
    // Incrementa a contagem de marcas da lateral direita
    void rightPassedInc();

private:
    std::string name;

    DataManager *dataManager;
};

#endif