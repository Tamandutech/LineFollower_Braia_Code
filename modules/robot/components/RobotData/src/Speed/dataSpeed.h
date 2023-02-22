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
    DataAbstract<int32_t> *EncMedia;

    // Variavel que contempla relacao de Revloucoes e reducao dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
    DataAbstract<uint16_t> *MPR;

    // Diâmetro das rodas
    DataAbstract<uint8_t> *WheelDiameter;

    DataAbstract<float> *initialaccelration; // aceleração inicial em rpm/s
    DataAbstract<float> *accelration; // aceleração em rpm/s
    DataAbstract<float> *desaccelration; // desaceleração em rpm/s
    
    // Restrições nos valores do PWM
    DataAbstract<int8_t> *max;
    DataAbstract<int8_t> *min;
    DataAbstract<int8_t> *base;

    
    DataAbstract<int16_t> *initialspeed; // Velocidade inicial em rpm
    DataAbstract<int16_t> *SetPointMap; // Setpoint translacional para o mapeamento em rpm

    //Setpoints translacionais para os tipos de trecho em rpm
    DataAbstract<int16_t> *Long_Line;
    DataAbstract<int16_t> *Medium_Line;
    DataAbstract<int16_t> *Short_Line;
    DataAbstract<int16_t> *Long_Curve;
    DataAbstract<int16_t> *Medium_Curve;
    DataAbstract<int16_t> *Short_Curve;
    DataAbstract<int16_t> *ZIGZAG;
    DataAbstract<int16_t> *Special_Track;

    // Velocidade para o modo Tunning
    DataAbstract<int16_t> *Tunning_speed;
    // Velocidade padrão do robô
    DataAbstract<int16_t> *Default_speed;

    // Velocidade atual da roda direita em PMW
    DataAbstract<float> *right;
    // Velocidade atual da roda esquerda em PMW
    DataAbstract<float> *left;

    // componentes da Velocidade total do robô 
    DataAbstract<float> *VelTrans;
    DataAbstract<float> *VelRot;

private:
    std::string name;
    const char *tag = "RobotData";

    DataManager *dataManager;
};

#endif