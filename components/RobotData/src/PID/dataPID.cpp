#include "dataPID.h"

dataPID::dataPID(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    // Inicializando os ponteiros para os tipos de dados.
    ESP_LOGD(tag, "Inicializando ponteiros para os tipos de dados");
    input = new DataAbstract<int16_t>("input", name);
    output = new DataAbstract<float>("output", name);
    setpoint = new DataAbstract<int16_t>("Setpoint", name);

    Kp_line = new DataAbstract<float>("Kp_line", name);
    Ki_line = new DataAbstract<float>("Ki_line", name);
    Kd_line = new DataAbstract<float>("Kd_line", name);

    Kp_curve = new DataAbstract<float>("Kp_curve", name);
    Ki_curve = new DataAbstract<float>("Ki_curve", name);
    Kd_curve = new DataAbstract<float>("Kd_curve", name);

    input->loadData();
    output->loadData();
    setpoint->loadData();
    Kp_line->loadData();
    Ki_line->loadData();
    Kd_line->loadData();
    Kp_curve->loadData();
    Ki_curve->loadData();
    Kd_curve->loadData();

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