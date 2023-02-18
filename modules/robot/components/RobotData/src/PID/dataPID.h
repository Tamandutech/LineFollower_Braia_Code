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

    DataAbstract<int16_t> *input;
    DataAbstract<float> *output;
    DataAbstract<int16_t> *setpoint; // salvar
    DataAbstract<float> *CorrectionFactor;
    DataAbstract<float> *CorrectionFactorLine;
    DataAbstract<float> *CorrectionFactorMediumCurve;
    DataAbstract<float> *CorrectionFactorShortCurve;
    DataAbstract<float> *CorrectionFactorLongCurve;
    DataAbstract<float> *CorrectionFactorZigZag;

    DataAbstract<bool> *UseKiVel;
    DataAbstract<bool> *UseKdIR;
    // Constantes do PID definidas pelo trecho da pista
    DataAbstract<float> *Kp(TrackState state);
    DataAbstract<float> *Ki(TrackState state);
    DataAbstract<float> *Kd(TrackState state);

private:
    std::string name;
    const char *tag = "RobotData";

    // Parâmetros do PID  
    DataAbstract<float> *Kp_std;
    DataAbstract<float> *Ki_std;
    DataAbstract<float> *Kd_std; 

    DataAbstract<float> *Kp_tunning; // salvar
    DataAbstract<float> *Ki_tunning; // salvar
    DataAbstract<float> *Kd_tunning; // salvar

    DataAbstract<float> *Kp_IRline; // salvar
    DataAbstract<float> *Kd_IRline; // salvar

    DataAbstract<float> *Kp_IRcurve; // salvar
    DataAbstract<float> *Kd_IRcurve; // salvar
    
    DataAbstract<float> *Kp_IRShortCurve; // Variável para limitar a velocidade rotacional do robô na curva curta
    DataAbstract<float> *Kd_IRShortCurve; // Variável para limitar a velocidade rotacional do robô na curva curta

    DataManager *dataManager;
};

#endif