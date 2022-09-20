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
    setpointLine = new DataAbstract<int16_t>("SetpointLine", name, 400);
    setpointCurve = new DataAbstract<int16_t>("SetpointCurve", name, 400);
    setpointMap = new DataAbstract<int16_t>("SetpointMap", name, 400);
    Krot = new DataAbstract<float>("Krot", name, 5);
    dataManager->registerParamData(setpointLine);
    dataManager->registerParamData(setpointCurve);
    dataManager->registerParamData(setpointMap);
    dataManager->registerParamData(Krot);

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

    VelTrans = new DataAbstract<float>("VelTrans", name, 0);
    dataManager->registerRuntimeData(VelTrans);

    ESP_LOGD(tag, "Ponteiros para os tipos de dados inicializados");
}

DataAbstract<float> *dataPID::Kp(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Kp_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter o Kp do PID, retornando valor para linha.", state);
        [[fallthrough]];
    case CAR_STOPPED:
        [[fallthrough]];
    case CAR_IN_LINE:
        return Kp_line;
        break;
    }
}

DataAbstract<float> *dataPID::Ki(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Ki_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter o Ki do PID, retornando valor para linha.", state);
        [[fallthrough]];
    case CAR_STOPPED:
        [[fallthrough]];
    case CAR_IN_LINE:
        return Ki_line;
        break;
    }
}

DataAbstract<float> *dataPID::Kd(CarState state)
{
    switch (state)
    {
    case CAR_IN_CURVE:
        return Kd_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter o Kd do PID, retornando valor para linha.", state);
        [[fallthrough]];
    case CAR_STOPPED:
        [[fallthrough]];
    case CAR_IN_LINE:
        return Kd_line;
        break;
    }
}