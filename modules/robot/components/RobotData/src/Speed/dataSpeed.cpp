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

    initialaccelration = new DataAbstract<float>("initial_accel", name, 2000);
    dataManager->registerParamData(initialaccelration); 
    accelration = new DataAbstract<float>("accel", name, 6000);
    dataManager->registerParamData(accelration);
    desaccelration = new DataAbstract<float>("desaccel", name, 6000);
    dataManager->registerParamData(desaccelration);

    WheelDiameter = new DataAbstract<uint8_t>("WheelDiameter", name, 0);
    dataManager->registerParamData(WheelDiameter);

    max = new DataAbstract<int8_t>("max", name, 100);
    dataManager->registerParamData(max);
    min = new DataAbstract<int8_t>("min", name, 0);
    dataManager->registerParamData(min);
    base = new DataAbstract<int8_t>("base", name, 0);
    dataManager->registerParamData(base);

    right = new DataAbstract<int8_t>("right", name, 0);
    dataManager->registerRuntimeData(right);
    left = new DataAbstract<int8_t>("left", name, 0);
    dataManager->registerRuntimeData(left);

    initialspeed = new DataAbstract<int16_t>("initial_speed", name, 1100);
    dataManager->registerParamData(initialspeed);

    //Setpoints translacionais para os tipos de trechos
    SetPointMap = new DataAbstract<int16_t>("Setpoint_Map", name, 600);
    dataManager->registerParamData(SetPointMap);
    Long_Line = new DataAbstract<int16_t>("Long_line", name, 1000);
    dataManager->registerParamData(Long_Line);
    Medium_Line = new DataAbstract<int16_t>("Medium_line", name, 1000);
    dataManager->registerParamData(Medium_Line);
    Short_Line = new DataAbstract<int16_t>("Short_line", name, 1000);
    dataManager->registerParamData(Short_Line);

    Long_Curve = new DataAbstract<int16_t>("Long_curve", name, 1000);
    dataManager->registerParamData(Long_Curve);
    Medium_Curve = new DataAbstract<int16_t>("Medium_curve", name, 1000);
    dataManager->registerParamData(Medium_Curve);
    Short_Curve = new DataAbstract<int16_t>("Short_curve", name, 1000);
    dataManager->registerParamData(Short_Curve);
    Tunning_speed = new DataAbstract<int16_t>("Tunning_speed", name, 1000);
    dataManager->registerParamData(Tunning_speed);
}