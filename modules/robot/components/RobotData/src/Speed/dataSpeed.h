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

    // Contagem atual dos encoders
    DataAbstract<int32_t> *EncRight;
    DataAbstract<int32_t> *EncLeft;
    DataAbstract<int32_t> *EncMedia;

    // Variavel que contempla relacao de Revloucoes e reducao dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
    DataAbstract<uint16_t> *MPR;

    // Diâmetro das rodas
    DataAbstract<uint8_t> *WheelDiameter;

    DataAbstract<uint16_t> *RobotDiameter;

    // Posicoes no plano cartesiano
    DataAbstract<float> *positionX;
    DataAbstract<float> *positionY; 

    DataAbstract<float> *initialaccelration; // aceleração inicial em rpm/s
    DataAbstract<float> *accelration; // aceleração em rpm/s
    DataAbstract<float> *desaccelration; // desaceleração em rpm/s
    DataAbstract<float> *MaxAccelration; // desaceleração em rpm/s
    DataAbstract<float> *DecelerationOffsetGain; // ganho para ajustar o offset para a desaceleração
    DataAbstract<float> *VariableAccelerationGain;
    // Velocidades em malha aberta
    DataAbstract<int8_t> *OpenLoopMaxSpeed;
    DataAbstract<int8_t> *OpenLoopMinSpeed;
    
    DataAbstract<float> *initialspeed; // Velocidade inicial em rpm

    DataAbstract<float> *MotorMaxSpeed; // em rpm

    //Setpoints translacionais para os tipos de trecho em rpm
    DataAbstract<float> *Long_Line;
    DataAbstract<float> *Medium_Line;
    DataAbstract<float> *Short_Line;
    DataAbstract<float> *XLong_Line;
    DataAbstract<float> *XLong_Curve; 
    DataAbstract<float> *Long_Curve;
    DataAbstract<float> *Medium_Curve;
    DataAbstract<float> *Short_Curve;
    DataAbstract<float> *ZIGZAG;
    DataAbstract<float> *Special_Track;

    DataAbstract<float> *linearSpeed; // Velocidade linear do robô
    DataAbstract<float> *RobotEstimatedLinearSpeed; // Velocidade linear estimada do robô
    DataAbstract<float> *Robot; // Velocidade linear estimada do robô

    // Velocidade para o modo Tunning
    DataAbstract<float> *Tunning_speed;
    // Velocidade padrão do robô
    DataAbstract<float> *Default_speed;

    // Velocidade atual da roda direita em PMW
    DataAbstract<float> *right;
    // Velocidade atual da roda esquerda em PMW
    DataAbstract<float> *left;

    // componentes da Velocidade total do robô 
    DataAbstract<float> *VelTrans;
    DataAbstract<float> *VelRot;

    DataAbstract<float> *PwmMean;
private:
    std::string name;
    const char *tag = "RobotData";

    DataManager *dataManager;
};

#endif