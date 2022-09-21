#include "dataSpeed.h"

dataSpeed::dataSpeed(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

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
    MPR = new DataAbstract<uint16_t>("MPR", name, 0);
    dataManager->registerParamData(MPR);

    accelration = new DataAbstract<float>("accel", name, 6000);
    dataManager->registerParamData(accelration);
    desaccelration = new DataAbstract<float>("desaccel", name, 6000);
    dataManager->registerParamData(desaccelration);

    WheelDiameter = new DataAbstract<uint8_t>("WheelDiameter", name, 0);
    dataManager->registerParamData(WheelDiameter);

    max = new DataAbstract<int8_t>("max", name, 0);
    min = new DataAbstract<int8_t>("min", name, 0);
    base = new DataAbstract<int8_t>("base", name, 0);

    right = new DataAbstract<int8_t>("right", name, 0);
    dataManager->registerRuntimeData(right);
    left = new DataAbstract<int8_t>("left", name, 0);
    dataManager->registerRuntimeData(left);

    // Linha
    max_line = new DataAbstract<int8_t>("max_line", name, 0);
    dataManager->registerParamData(max_line);
    min_line = new DataAbstract<int8_t>("min_line", name, 0);
    dataManager->registerParamData(min_line);
    base_line = new DataAbstract<int8_t>("base_line", name, 0);
    dataManager->registerParamData(base_line);

    // Curva
    max_curve = new DataAbstract<int8_t>("max_curve", name, 0);
    dataManager->registerParamData(max_curve);
    min_curve = new DataAbstract<int8_t>("min_curve", name, 0);
    dataManager->registerParamData(min_curve);
    base_curve = new DataAbstract<int8_t>("base_curve", name, 0);
    dataManager->registerParamData(base_curve);

    // Mapeamento
    max_mapping = new DataAbstract<int8_t>("max_mapping", name, 0);
    dataManager->registerParamData(max_mapping);
    min_mapping = new DataAbstract<int8_t>("min_mapping", name, 0);
    dataManager->registerParamData(min_mapping);
    base_mapping = new DataAbstract<int8_t>("base_mapping", name, 0);
    dataManager->registerParamData(base_mapping);

    //Setpoints translacionais para os tipos de trechos
    Long_Line = new DataAbstract<uint16_t>("Long_line", name, 1000);
    dataManager->registerParamData(Long_Line);
    Medium_Line = new DataAbstract<uint16_t>("Medium_line", name, 1000);
    dataManager->registerParamData(Medium_Line);
    Short_Line = new DataAbstract<uint16_t>("Short_line", name, 1000);
    dataManager->registerParamData(Short_Line);

    Long_Curve = new DataAbstract<uint16_t>("Long_curve", name, 1000);
    dataManager->registerParamData(Long_Curve);
    Medium_Curve = new DataAbstract<uint16_t>("Medium_curve", name, 1000);
    dataManager->registerParamData(Medium_Curve);
    Short_Curve = new DataAbstract<uint16_t>("Short_curve", name, 1000);
    dataManager->registerParamData(Short_Curve);

    // Componentes da velocidade total
    VelTrans = new DataAbstract<float>("VelTrans", name, 0);
    dataManager->registerRuntimeData(VelTrans);

    VelRot = new DataAbstract<float>("VelRot", name, 0);
    dataManager->registerRuntimeData(VelRot);
    
}

void dataSpeed::setToLine()
{
    ESP_LOGD(tag, "Setando para linha");
    max = max_line;
    min = min_line;
    base = base_line;
}

void dataSpeed::setToCurve()
{
    ESP_LOGD(tag, "Setando para curva");
    max = max_curve;
    min = min_curve;
    base = base_curve;
}

void dataSpeed::setToMapping()
{
    ESP_LOGD(tag, "Setando para mapeamento");
    max = max_mapping;
    min = min_mapping;
    base = base_mapping;
}