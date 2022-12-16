#include "dataPID.h"

dataPID::dataPID(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

    // Inicializando os ponteiros para os tipos de dados.
    ESP_LOGD(tag, "Inicializando ponteiros para os tipos de dados");

    input = new DataAbstract<int16_t>("input", name);
    output = new DataAbstract<float>("output", name);
    setpoint = new DataAbstract<int16_t>("Setpoint", name);
    
    if(name == "PIDRot")
    {
        Krot = new DataAbstract<float>("Krot", name, 5);
        KrotLongCurve = new DataAbstract<float>("KrotLongCurve", name, 4.6);
        KrotMediumCurve = new DataAbstract<float>("KrotMediumCurve", name, 4.6);
        KrotShortCurve = new DataAbstract<float>("KrotShortCurve", name, 4.6);
        dataManager->registerParamData(Krot);
        dataManager->registerParamData(KrotLongCurve);
        dataManager->registerParamData(KrotMediumCurve);
        dataManager->registerParamData(KrotShortCurve);
    }

    Kp_tunning = new DataAbstract<float>("Kp_tunning", name);
    dataManager->registerParamData(Kp_tunning);
    Ki_tunning = new DataAbstract<float>("Ki_tunning", name);
    dataManager->registerParamData(Ki_tunning);
    Kd_tunning = new DataAbstract<float>("Kd_tunning", name);
    dataManager->registerParamData(Kd_tunning);

    Kp_line = new DataAbstract<float>("Kp_line", name);
    dataManager->registerParamData(Kp_line);
    Ki_line = new DataAbstract<float>("Ki_line", name);
    dataManager->registerParamData(Ki_line);
    Kd_line = new DataAbstract<float>("Kd_line", name);
    dataManager->registerParamData(Kd_line);

    Kp_curve = new DataAbstract<float>("Kp_curve", name);
    dataManager->registerParamData(Kp_curve);
    Ki_curve = new DataAbstract<float>("Ki_curve", name);
    dataManager->registerParamData(Ki_curve);
    Kd_curve = new DataAbstract<float>("Kd_curve", name);
    dataManager->registerParamData(Kd_curve);

    if(name == "PIDVel")
    {
        CorrectionFactor = new DataAbstract<float>("FatorCorrecao", name);
        dataManager->registerParamData(CorrectionFactor);
    }
    
    ESP_LOGD(tag, "Ponteiros para os tipos de dados inicializados");
}

DataAbstract<float> *dataPID::Kp(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Kp_curve;
        break;
    case CAR_TUNING:
        return Kp_tunning;
        break;
    case CAR_IN_LINE:
        return Kp_line;
        break;
    default:
        ESP_LOGE(tag, "Estado do Robô inválido para esse método: %d para obter o Kp do PID, retornando valor para linha.", state);
        break;
    }
    return nullptr;
}

DataAbstract<float> *dataPID::Ki(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Ki_curve;
        break;
    case CAR_TUNING:
        return Ki_tunning;
        break;
    case CAR_IN_LINE:
        return Ki_line;
        break;
    default:
        ESP_LOGE(tag, "Estado do Robô inválido para esse método: %d para obter o Ki do PID, retornando valor para linha.", state);
        break;
    }
    return nullptr;
}

DataAbstract<float> *dataPID::Kd(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Kd_curve;
        break;
    case CAR_TUNING:
        return Kd_tunning;
        break;
    case CAR_IN_LINE:
        return Kd_line;
        break;
    default:
        ESP_LOGE(tag, "Estado do Robô inválido para esse método: %d para obter o Kd do PID, retornando valor para linha.", state);
        break;
    }
    return nullptr;
}