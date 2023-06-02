#ifndef DATA_PID_H
#define DATA_PID_H

#define DEFAULT_SETPOINT 3500

#define DEFAULT_KP_LINE 3500
#define DEFAULT_KP_CURVE 3500

#define DEFAULT_KI_LINE 3500
#define DEFAULT_KI_CURVE 3500

#define DEFAULT_KD_LINE 3500
#define DEFAULT_KD_CURVE 3500

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

class dataPID
{
public:
    dataPID(std::string name = "dataPID");

    DataAbstract<float> *input;
    DataAbstract<float> *output;
    DataAbstract<float> *P_output;
    DataAbstract<float> *I_output;
    DataAbstract<float> *D_output;
    DataAbstract<float> *setpoint;
    DataAbstract<float> *erro;
    DataAbstract<float> *erroquad;

    DataAbstract<bool> *UseKiVel;
    DataAbstract<bool> *UseKdIR;
    // Constantes do PID definidas pelo trecho da pista
    DataAbstract<double> *Kp(TrackState state);
    DataAbstract<double> *Ki(TrackState state);
    DataAbstract<double> *Kd(TrackState state);


private:
    std::string name;
    const char *tag = "RobotData";

    // Parâmetros do PID  
    DataAbstract<double> *Kp_std;
    DataAbstract<double> *Ki_std;
    DataAbstract<double> *Kd_std; 

    // Parâmetros do PID para diferentes trechos da pista
    DataAbstract<double> *Kp_tunning; // salvar
    DataAbstract<double> *Ki_tunning; // salvar
    DataAbstract<double> *Kd_tunning; // salvar

    DataAbstract<double> *Kp_IRline; // salvar
    DataAbstract<double> *Kd_IRline; // salvar

    DataAbstract<double> *Kp_IRcurve; // salvar
    DataAbstract<double> *Kd_IRcurve; // salvar
    
    DataAbstract<double> *Kp_IRShortCurve; 
    DataAbstract<double> *Kd_IRShortCurve; 

    DataAbstract<double> *Kp_IRZigZag;
    DataAbstract<double> *Kd_IRZigZag; 

    DataAbstract<double> *Kp_IRXLongLine; // Pista Maua
    DataAbstract<double> *Kd_IRXLongLine; 

    DataAbstract<double> *Kp_IRLongCurve;
    DataAbstract<double> *Kd_IRLongCurve; 

    DataAbstract<double> *Kp_IRXLongCurve; // Pista Maua
    DataAbstract<double> *Kd_IRXLongCurve;

    DataManager *dataManager;
};

#endif