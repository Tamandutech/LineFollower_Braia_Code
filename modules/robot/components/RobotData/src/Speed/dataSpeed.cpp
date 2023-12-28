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
    dataManager->registerRuntimeData(EncRight);
    EncLeft = new DataAbstract<int32_t>("EncLeft", name, 0);
    dataManager->registerRuntimeData(EncLeft);
    EncMedia = new DataAbstract<int32_t>("EncMedia", name, 0);
    dataManager->registerRuntimeData(EncMedia);

    positionX = new DataAbstract<float>("positionX", name, 0);
    dataManager->registerRuntimeData(positionX);
    positionY = new DataAbstract<float>("positionY", name, 0);
    dataManager->registerRuntimeData(positionY);

    /*
     * Variavel que contempla relacao de Revloucoes e reducao
     * dos motores, entrada eh ((Qtd de pulsos para uma volta) * (Reducao do motor))
     * */
    MPR = new DataAbstract<uint16_t>("MPR", name, 0);
    dataManager->registerParamData(MPR);


    WheelDiameter = new DataAbstract<uint8_t>("WheelDiameter", name, 0);
    dataManager->registerParamData(WheelDiameter);
    RobotDiameter = new DataAbstract<uint16_t>("RobotDiameter", name, 112); //mm
    dataManager->registerParamData(RobotDiameter);

    max = new DataAbstract<int8_t>("max", name, 100);
    dataManager->registerParamData(max);
    min = new DataAbstract<int8_t>("min", name, 0);
    dataManager->registerParamData(min);
    base = new DataAbstract<int8_t>("base", name, 0);
    dataManager->registerParamData(base);

    right = new DataAbstract<float>("right", name, 0);
    dataManager->registerRuntimeData(right);
    left = new DataAbstract<float>("left", name, 0);
    dataManager->registerRuntimeData(left);

    initialaccelration = new DataAbstract<float>("PWM_initial_accel", name, 2000);
    dataManager->registerParamData(initialaccelration); 
    accelration = new DataAbstract<float>("PWM_accel", name, 6000);
    dataManager->registerParamData(accelration);
    desaccelration = new DataAbstract<float>("PWM_desaccel", name, 6000);
    dataManager->registerParamData(desaccelration);

    initialspeed = new DataAbstract<float>("PWM_initial_speed", name, 1100);
    dataManager->registerParamData(initialspeed);

<<<<<<< HEAD
        //Setpoints translacionais para os tipos de trechos
        SetPointMap = new DataAbstract<float>("PWM_Setpoint_Map", name, 600);
        dataManager->registerParamData(SetPointMap);
        XLong_Line = new DataAbstract<float>("PWM_XLong_line", name, 1000);
        dataManager->registerParamData(XLong_Line);
        Long_Line = new DataAbstract<float>("PWM_Long_line", name, 1000);
        dataManager->registerParamData(Long_Line);
        Medium_Line = new DataAbstract<float>("PWM_Medium_line", name, 1000);
        dataManager->registerParamData(Medium_Line);
        Short_Line = new DataAbstract<float>("PWM_Short_line", name, 1000);
        dataManager->registerParamData(Short_Line);
        XLong_Curve = new DataAbstract<float>("PWM_XLong_curve", name, 1000);
        dataManager->registerParamData(XLong_Curve);
        Long_Curve = new DataAbstract<float>("PWM_Long_curve", name, 1000);
        dataManager->registerParamData(Long_Curve);
        Medium_Curve = new DataAbstract<float>("PWM_Medium_curve", name, 1000);
        dataManager->registerParamData(Medium_Curve);
        Short_Curve = new DataAbstract<float>("PWM_Short_curve", name, 1000);
        dataManager->registerParamData(Short_Curve);
        Zig_Zag = new DataAbstract<float>("PWM_ZigZag", name, 1000);
        dataManager->registerParamData(Zig_Zag);
        Special_Track = new DataAbstract<float>("PWM_SpecialTrack", name, 1000);
        dataManager->registerParamData(Special_Track);
        Default_speed = new DataAbstract<float>("PWM_DefaultSpeed", name, 1000);
        dataManager->registerParamData(Default_speed);
        Tunning_speed = new DataAbstract<float>("PWM_Tunning_speed", name, 1000);
        dataManager->registerParamData(Tunning_speed);
    }
    else
    {
        initialaccelration = new DataAbstract<float>("initial_accel", name, 2000);
        dataManager->registerParamData(initialaccelration); 
        accelration = new DataAbstract<float>("accel", name, 6000);
        dataManager->registerParamData(accelration);
        desaccelration = new DataAbstract<float>("desaccel", name, 6000);
        dataManager->registerParamData(desaccelration);

        initialspeed = new DataAbstract<float>("initial_speed", name, 1100);
        dataManager->registerParamData(initialspeed);

        //Setpoints translacionais para os tipos de trechos
        SetPointMap = new DataAbstract<float>("Setpoint_Map", name, 600);
        dataManager->registerParamData(SetPointMap);
        Long_Line = new DataAbstract<float>("Long_line", name, 1000);
        dataManager->registerParamData(Long_Line);
        Medium_Line = new DataAbstract<float>("Medium_line", name, 1000);
        dataManager->registerParamData(Medium_Line);
        Short_Line = new DataAbstract<float>("Short_line", name, 1000);
        dataManager->registerParamData(Short_Line);

        Long_Curve = new DataAbstract<float>("Long_curve", name, 1000);
        dataManager->registerParamData(Long_Curve);
        Medium_Curve = new DataAbstract<float>("Medium_curve", name, 1000);
        dataManager->registerParamData(Medium_Curve);
        Short_Curve = new DataAbstract<float>("Short_curve", name, 1000);
        dataManager->registerParamData(Short_Curve);
        Zig_Zag = new DataAbstract<float>("ZigZag", name, 1000);
        dataManager->registerParamData(ZIGZAG);
        Special_Track = new DataAbstract<float>("SpecialTrack", name, 1000);
        dataManager->registerParamData(Special_Track);
        Default_speed = new DataAbstract<float>("DefaultSpeed", name, 1000);
        dataManager->registerParamData(Default_speed);
        Tunning_speed = new DataAbstract<float>("Tunning_speed", name, 1000);
        dataManager->registerParamData(Tunning_speed);
    }
=======
    //Setpoints translacionais para os tipos de trechos
    SetPointMap = new DataAbstract<float>("PWM_Setpoint_Map", name, 600);
    dataManager->registerParamData(SetPointMap);
    XLong_Line = new DataAbstract<float>("PWM_XLong_line", name, 1000);
    dataManager->registerParamData(XLong_Line);
    Long_Line = new DataAbstract<float>("PWM_Long_line", name, 1000);
    dataManager->registerParamData(Long_Line);
    Medium_Line = new DataAbstract<float>("PWM_Medium_line", name, 1000);
    dataManager->registerParamData(Medium_Line);
    Short_Line = new DataAbstract<float>("PWM_Short_line", name, 1000);
    dataManager->registerParamData(Short_Line);
    XLong_Curve = new DataAbstract<float>("PWM_XLong_curve", name, 1000);
    dataManager->registerParamData(XLong_Curve);
    Long_Curve = new DataAbstract<float>("PWM_Long_curve", name, 1000);
    dataManager->registerParamData(Long_Curve);
    Medium_Curve = new DataAbstract<float>("PWM_Medium_curve", name, 1000);
    dataManager->registerParamData(Medium_Curve);
    Short_Curve = new DataAbstract<float>("PWM_Short_curve", name, 1000);
    dataManager->registerParamData(Short_Curve);
    ZIGZAG = new DataAbstract<float>("PWM_ZigZag", name, 1000);
    dataManager->registerParamData(ZIGZAG);
    Special_Track = new DataAbstract<float>("PWM_SpecialTrack", name, 1000);
    dataManager->registerParamData(Special_Track);
    Default_speed = new DataAbstract<float>("PWM_DefaultSpeed", name, 1000);
    dataManager->registerParamData(Default_speed);
    Tunning_speed = new DataAbstract<float>("PWM_Tunning_speed", name, 1000);
    dataManager->registerParamData(Tunning_speed);
>>>>>>> cc9d32ab8ad251ac5989ed6aeadc68b6a522c307

    // Componentes da velocidade total
    VelTrans = new DataAbstract<float>("VelTrans", name, 0);
    dataManager->registerRuntimeData(VelTrans);

    VelRot = new DataAbstract<float>("VelRot", name, 0);
    dataManager->registerRuntimeData(VelRot);

    // Velocidade linear desejada no momento
    CalculatedSpeed = new DataAbstract<float>("CalculatedSpeed", name, 0);
    dataManager->registerRuntimeData(CalculatedSpeed);


}