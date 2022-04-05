#ifndef DATA_ENUMS_H
#define DATA_ENUMS_H

struct MapData{
    int32_t MapTime;
    int32_t MapEncMedia;
    int32_t MapEncLeft;
    int32_t MapEncRight;
    int32_t MapStatus;
};
enum CarState
{
    CAR_IN_CURVE,
    CAR_IN_LINE,
    CAR_STOPPED
};

enum DataFunction
{
    RETORNO_OK,
    RETORNO_ERRO,
    RETORNO_ERRO_GENERICO,
    RETORNO_ARGUMENTO_INVALIDO,
    RETORNO_VARIAVEL_OCUPADA
};

enum ProtocolCodes
{
    MapDataSend,
    dataPidSend,
    dataSpeedSend,
    RobotStatusSend,
    ParametersSend,
    CMDTXT,
    MarkData

};

struct CarParameters{
    float KpRotMapLine = 0.27;
    float KpVelMapLine = 0.05;

    float KdRotMapLine = 0.0001;
    float KdVelMapLine = 0;
    float VelTargetMap = 500;

    float KpRotMapCurve = 0.27;
    float KpVelMapCurve = 0.05;

    float KdRotMapCurve = 0.0001;
    float KdVelMapCurve = 0;

    float KpRotRunLine = 0.27;
    float KpVelRunLine = 0.05;

    float KdRotRunLine = 0.0001;
    float KdVelRunLine = 0;
    float VelTargetRunLine = 900;

    float KpRotRunCurve = 0.27;
    float KpVelRunCurve = 0.05;

    float KdRotRunCurve = 0.0001;
    float KdVelRunCurve = 0.0001;
    float VelTargetRunCurve = 500;

    int SpeedBaseMapLine = 25;
    int SpeedMaxMapLine = 50;
    int SpeedMinMapLine = 5;

    int SpeedBaseMapCurve = 25;
    int SpeedMaxMapCurve = 50;
    int SpeedMinMapCurve = 5;

    int SpeedBaseRunLine = 40;
    int SpeedMaxRunLine = 70;
    int SpeedMinRunLine = 5;

    int SpeedBaseRunCurve = 20;
    int SpeedMaxRunCurve = 50;
    int SpeedMinRunCurve = 5;
};

struct PacketData{
    uint8_t cmd;  // Comando
    uint8_t version; // Versão do comando
    uint16_t packetsToReceive;  // Quantidade de pacotes para receber ainda
    uint16_t size;  // Tamanho total do dado que será enviado pelos pacotes
    uint8_t packetsize; // Tamanho do dado contido no pacote
    uint8_t data[230]; //Mensagem
};

struct SLatMarks{
    uint16_t leftMarks;
    uint16_t rightMarks;
    struct MapData MarksData[70];
    uint16_t TotalLeftMarks;
    int32_t InitialMark;
    int32_t FinalMark;
    bool MapFinished;
};
#endif