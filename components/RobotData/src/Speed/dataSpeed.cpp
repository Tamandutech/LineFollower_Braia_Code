#include "dataSpeed.h"

dataSpeed::dataSpeed(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    // Valocidades atuais
    RPMRight_inst = new DataAbstract<int16_t>("RPMRight_inst", name, 0);
    RPMLeft_inst = new DataAbstract<int16_t>("RPMLeft_inst", name, 0);
    RPMCar_media = new DataAbstract<int16_t>("RPMCar_media", name, 0);

    // Contagem atual dos encoders
    EncRight = new DataAbstract<int32_t>("EncRight", name, 0);
    EncLeft = new DataAbstract<int32_t>("EncLeft", name, 0);

    /*
     * Variavel que contempla relacao de Revloucoes e reducao
     * dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
     * */
    MPR_MotEsq = new DataAbstract<uint16_t>("MPR_MotEsq", name, 0);
    MPR_MotDir = new DataAbstract<uint16_t>("MPR_MotDir", name, 0);

    // Linha
    right_line = new DataAbstract<int8_t>("right_line", name, 0);
    left_line = new DataAbstract<int8_t>("left_line", name, 0);
    max_line = new DataAbstract<int8_t>("max_line", name, 0);
    min_line = new DataAbstract<int8_t>("min_line", name, 0);
    base_line = new DataAbstract<int8_t>("base_line", name, 0);

    // Curva
    right_curve = new DataAbstract<int8_t>("right_curve", name, 0);
    left_curve = new DataAbstract<int8_t>("left_curve", name, 0);
    max_curve = new DataAbstract<int8_t>("max_curve", name, 0);
    min_curve = new DataAbstract<int8_t>("min_curve", name, 0);
    base_curve = new DataAbstract<int8_t>("base_curve", name, 0);

    // Carregar todos os dados
    RPMRight_inst->loadData();
    RPMLeft_inst->loadData();
    RPMCar_media->loadData();
    EncRight->loadData();
    EncLeft->loadData();
    MPR_MotEsq->loadData();
    MPR_MotDir->loadData();
    right_line->loadData();
    left_line->loadData();
    max_line->loadData();
    min_line->loadData();
    base_line->loadData();
    right_curve->loadData();
    left_curve->loadData();
    max_curve->loadData();
    min_curve->loadData();
    base_curve->loadData();
    
}

// Metodos de valores variaveis

DataAbstract<int8_t> *dataSpeed::SpeedLeft(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_CURVE:
        return this->left_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter a velocidade da esqueda, retornando valor para linha.", carState);
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return this->left_line;
        break;
    }
}
DataAbstract<int8_t> *dataSpeed::SpeedRight(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_CURVE:
        return this->right_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter a velocidade da direita, retornando valor para linha.", carState);
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return this->right_line;
        break;
    }
}
DataAbstract<int8_t> *dataSpeed::SpeedMax(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_CURVE:
        return this->max_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter a velocidade máxima, retornando valor para linha.", carState);
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return this->max_line;
        break;
    }
}
DataAbstract<int8_t> *dataSpeed::SpeedMin(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_CURVE:
        return this->min_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter a velocidade minima, retornando valor para linha.", carState);
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return this->min_line;
        break;
    }
}
DataAbstract<int8_t> *dataSpeed::SpeedBase(CarState carState)
{
    switch (carState)
    {
    case CAR_IN_CURVE:
        return this->base_curve;
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido: %d para obter a velocidade da esqueda, retornando valor para linha.", carState);
    case CAR_STOPPED:
    case CAR_IN_LINE:
        return this->base_line;
        break;
    }
}
