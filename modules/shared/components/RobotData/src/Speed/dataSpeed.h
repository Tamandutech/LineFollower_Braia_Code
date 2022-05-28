#ifndef DATA_SPEED_H
#define DATA_SPEED_H

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "dataEnums.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"
#include "DataManager.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class dataSpeed
{
public:
    dataSpeed(std::string name = "dataSpeed");

    // Valocidades atuais
    DataAbstract<int16_t> *RPMRight_inst;
    DataAbstract<int16_t> *RPMLeft_inst;
    DataAbstract<int16_t> *RPMCar_media;

    // Contagem atual dos encoders
    DataAbstract<int32_t> *EncRight;
    DataAbstract<int32_t> *EncLeft;

    // Variavel que contempla relacao de Revloucoes e reducao dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
    DataAbstract<uint16_t> *MPR;

    // Diâmetro das rodas
    DataAbstract<uint8_t> *WheelDiameter;

    DataAbstract<float> *accelration; // aceleração em rpm/s
    DataAbstract<int8_t> *max;
    DataAbstract<int8_t> *min;
    DataAbstract<int8_t> *base;

    // Velocidade atual da roda direita em PMW
    DataAbstract<int8_t> *right;
    // Velocidade atual da roda esquerda em PMW
    DataAbstract<int8_t> *left;

    void setToLine();
    void setToCurve();
    void setToMapping();

private:
    std::string name;
    const char *tag = "RobotData";

    DataManager *dataManager;

    // Linha
    DataAbstract<int8_t> *max_line;  // salvar
    DataAbstract<int8_t> *min_line;  // salvar
    DataAbstract<int8_t> *base_line; // salvar

    // Curva
    DataAbstract<int8_t> *max_curve;  // salvar
    DataAbstract<int8_t> *min_curve;  // salvar
    DataAbstract<int8_t> *base_curve; // salvar

    // Mapeamento
    DataAbstract<int8_t> *max_mapping;  // salvar
    DataAbstract<int8_t> *min_mapping;  // salvar
    DataAbstract<int8_t> *base_mapping; // salvar
};

#endif