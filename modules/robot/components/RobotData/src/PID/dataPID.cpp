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

    Kp_std = new DataAbstract<double>("Kp_std", name, 0);
    dataManager->registerParamData(Kp_std);
    Kd_std = new DataAbstract<double>("Kd_std", name, 0);
    dataManager->registerParamData(Kd_std);
    Nfilter = new DataAbstract<double>("N_filter", name, 0);
    dataManager->registerParamData(Nfilter);
    if(name == "PIDVel" || name == "PIDRot")
    {
        Ki_std = new DataAbstract<double>("Ki_std", name, 0);
        dataManager->registerParamData(Ki_std);
        Ki_tunning = new DataAbstract<double>("Ki_tunning", name);
        dataManager->registerParamData(Ki_tunning);
    }

    Kp_tunning = new DataAbstract<double>("Kp_tunning", name);
    dataManager->registerParamData(Kp_tunning);
    Kd_tunning = new DataAbstract<double>("Kd_tunning", name);
    dataManager->registerParamData(Kd_tunning);



    if(name == "PIDIR" || name == "PIDClassic")
    {
        Kp_IRline = new DataAbstract<double>("Kp_IRline", name);
        dataManager->registerParamData(Kp_IRline);
        Kd_IRline = new DataAbstract<double>("Kd_IRline", name);
        dataManager->registerParamData(Kd_IRline);
        Kp_IRcurve = new DataAbstract<double>("Kp_IRcurve", name);
        dataManager->registerParamData(Kp_IRcurve);
        Kd_IRcurve = new DataAbstract<double>("Kd_IRcurve", name);
        dataManager->registerParamData(Kd_IRcurve);
        Kp_IRShortCurve = new DataAbstract<double>("Kp_IRShortCurve", name, 4.5);
        dataManager->registerParamData(Kp_IRShortCurve);
        Kd_IRShortCurve = new DataAbstract<double>("Kd_IRShortCurve", name, 4.5);
        dataManager->registerParamData(Kd_IRShortCurve);
        Kp_IRZigZag = new DataAbstract<double>("Kp_IRZigZag", name, 0.03);
        dataManager->registerParamData(Kp_IRZigZag);
        Kd_IRZigZag = new DataAbstract<double>("Kd_IRZigZag", name, 0.94);
        dataManager->registerParamData(Kd_IRZigZag);
        Kp_IRXLongLine = new DataAbstract<double>("Kp_IRXLongLine", name, 0.03);
        dataManager->registerParamData(Kp_IRXLongLine);
        Kd_IRXLongLine = new DataAbstract<double>("Kd_IRXLongLine", name, 0.94);
        dataManager->registerParamData(Kd_IRXLongLine);
        Kp_IRLongCurve = new DataAbstract<double>("Kp_IRLongCurve", name, 0.03);
        dataManager->registerParamData(Kp_IRLongCurve);
        Kd_IRLongCurve = new DataAbstract<double>("Kd_IRLongCurve", name, 0.94);
        dataManager->registerParamData(Kd_IRLongCurve);

        Kp_IRXLongCurve = new DataAbstract<double>("Kp_IRXLongCurve", name, 0.03);
        dataManager->registerParamData(Kp_IRXLongCurve);
        Kd_IRXLongCurve = new DataAbstract<double>("Kd_IRXLongCurve", name, 0.94);
        dataManager->registerParamData(Kd_IRXLongCurve);

        UseKdIR = new DataAbstract<bool>("UseKdIR", name, true);
        dataManager->registerParamData(UseKdIR);
    }

    if(name == "PIDVel")
    {

        UseKiVel = new DataAbstract<bool>("UseKiVel", name, false);
        dataManager->registerParamData(UseKiVel);
    }

    ESP_LOGD(tag, "Ponteiros para os tipos de dados inicializados");
}

DataAbstract<double> *dataPID::Kp(TrackState state)
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

DataAbstract<double> *dataPID::Ki(TrackState state)
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

DataAbstract<double> *dataPID::Kd(TrackState state)
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