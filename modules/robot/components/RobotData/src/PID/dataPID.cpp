#include "dataPID.h"

dataPID::dataPID(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

    // Inicializando os ponteiros para os tipos de dados.
    ESP_LOGD(tag, "Inicializando ponteiros para os tipos de dados");

    input = new DataAbstract<float>("input", name, 0);
    dataManager->registerRuntimeData(input);
    output = new DataAbstract<float>("output", name, 0);
    dataManager->registerRuntimeData(output);
    P_output = new DataAbstract<float>("P_output", name, 0);
    dataManager->registerRuntimeData(P_output);
    I_output = new DataAbstract<float>("I_output", name, 0);
    dataManager->registerRuntimeData(I_output);
    D_output = new DataAbstract<float>("D_output", name, 0);
    dataManager->registerRuntimeData(D_output);
    setpoint = new DataAbstract<float>("Setpoint", name, 0);
    dataManager->registerRuntimeData(setpoint);
    erro = new DataAbstract<float>("erro", name, 0);
    dataManager->registerRuntimeData(erro);
    erroquad = new DataAbstract<float>("erroquad", name, 0);
    dataManager->registerRuntimeData(erroquad);

    Kp_default = new DataAbstract<double>("Kp_default", name, 0);
    dataManager->registerParamData(Kp_default);
    Kd_default = new DataAbstract<double>("Kd_default", name, 0);
    dataManager->registerParamData(Kd_default);
    Ki_default = new DataAbstract<double>("Ki_default", name, 0);
    dataManager->registerParamData(Ki_default);

    Kp_tunning = new DataAbstract<double>("Kp_tunning", name);
    dataManager->registerParamData(Kp_tunning);
    Kd_tunning = new DataAbstract<double>("Kd_tunning", name);
    dataManager->registerParamData(Kd_tunning);
    Ki_tunning = new DataAbstract<double>("Ki_tunning", name);
    dataManager->registerParamData(Ki_tunning);

    Kp_line = new DataAbstract<double>("Kp_line", name);
    dataManager->registerParamData(Kp_line);
    Kd_line = new DataAbstract<double>("Kd_line", name);
    dataManager->registerParamData(Kd_line);
    Kp_curve = new DataAbstract<double>("Kp_curve", name);
    dataManager->registerParamData(Kp_curve);
    Kd_curve = new DataAbstract<double>("Kd_curve", name);
    dataManager->registerParamData(Kd_curve);
    Kp_ZigZag = new DataAbstract<double>("Kp_ZigZag", name, 0.03);
    dataManager->registerParamData(Kp_ZigZag);
    Kd_ZigZag = new DataAbstract<double>("Kd_ZigZag", name, 0.94);
    dataManager->registerParamData(Kd_ZigZag);

    ESP_LOGD(tag, "Ponteiros para os tipos de dados inicializados");
}

DataAbstract<double> *dataPID::Kp(TrackSegment state)
{
    if(name == "PIDVel" || name == "PIDRot")
    {
        if(state == TUNING)
        {
            return Kp_tunning;
        }
        else
        {
            return Kp_std;
        }
    }
    else if(name == "PIDIR" || name=="PIDClassic")
    {
        if(state == SHORT_LINE || state == MEDIUM_LINE || state == LONG_LINE)
        {
            return Kp_IRline;
        }
        else if(state == SHORT_CURVE)
        {
            return Kp_IRShortCurve;
        }
        else if(state == XLONG_LINE)
        {
            return Kp_IRXLongLine;
        }
        else if(state == ZIGZAG)
        {
            return Kp_IRZigZag;
        }
        else if(state == MEDIUM_CURVE)
        {
            return Kp_IRcurve;
        }
        else if(state == XLONG_CURVE)
        {
            return Kp_IRXLongCurve;
        }
        else if(state == LONG_CURVE)
        {
            return Kp_IRLongCurve;
        }
        else if(state == TUNING)
        {
            return Kp_tunning;
        }
        else
        {
            return Kp_std;
        }
        
    }
    ESP_LOGE(tag, "Estado do Robô ou Objeto PID inválido para esse método: %s:%d para obter o Kp do PID, retornando valor null.", name.c_str(),state);
    return nullptr;
}

DataAbstract<double> *dataPID::Ki(TrackSegment state)
{
    if(name == "PIDVel" || name == "PIDRot")
    {
        if(state == TUNING)
        {
            return Ki_tunning;
        }
        else
        {
            return Ki_std;
        }
    }
    ESP_LOGE(tag, "Estado do Robô ou Objeto PID inválido para esse método: %s:%d para obter o Ki do PID, retornando valor null.", name.c_str(),state);
    return nullptr;
}

DataAbstract<double> *dataPID::Kd(TrackSegment state)
{

    if(name == "PIDVel" || name == "PIDRot")
    {
        if(state == TUNING)
        {
            return Kd_tunning;
        }
        else
        {
            return Kd_std;
        }
    }
    else if(name == "PIDIR" || name== "PIDClassic")
    {
        if(state == SHORT_LINE || state == MEDIUM_LINE || state == LONG_LINE)
        {
            return Kd_IRline;
        }
        else if(state == SHORT_CURVE)
        {
            return Kd_IRShortCurve;
        }
        else if(state == ZIGZAG)
        {
            return Kd_IRZigZag;
        }
        else if(state == XLONG_LINE)
        {
            return Kd_IRXLongLine;
        }
        else if(state == XLONG_CURVE)
        {
            return Kd_IRXLongCurve;
        }
        else if(state == MEDIUM_CURVE)
        {
            return Kd_IRcurve;
        }
        else if(state == LONG_CURVE)
        {
            return Kd_IRLongCurve;
        }
        else if(state == TUNING)
        {
            return Kd_tunning;
        }
        else
        {
            return Kd_std;
        }
        
    }
    ESP_LOGE(tag, "Estado do Robô ou Objeto PID inválido para esse método: %s:%d para obter o Kd do PID, retornando valor null.", name.c_str(),state);
    return nullptr;
}