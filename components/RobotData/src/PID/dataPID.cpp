#include "dataPID.h"

dataPID::dataPID(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    // Inicializando os ponteiros para os tipos de dados.
    ESP_LOGD(tag, "Inicializando ponteiros para os tipos de dados");
    input = new DataAbstract<int16_t>("input");
    output = new DataAbstract<float>("output");
    setpoint = new DataAbstract<int16_t>("Setpoint");

    Kp_line = new DataAbstract<float>("Kp_line");
    Ki_line = new DataAbstract<float>("Ki_line");
    Kd_line = new DataAbstract<float>("Kd_line");

    Kp_curve = new DataAbstract<float>("Kp_curve");
    Ki_curve = new DataAbstract<float>("Ki_curve");
    Kd_curve = new DataAbstract<float>("Kd_curve");

    input->saveData("input");
    output->saveData("output");
    setpoint->saveData("Setpoint");
    Kp_line->saveData("Kp_line");
    Ki_line->saveData("Ki_line");
    Kd_line->saveData("Kd_line");
    Kp_curve->saveData("Kp_curve");
    Ki_curve->saveData("Ki_curve");
    Kd_curve->saveData("Kd_curve");
    
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
    case CAR_STOPPED:
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
    case CAR_STOPPED:
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
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return Kd_line;
        break;
    }
}