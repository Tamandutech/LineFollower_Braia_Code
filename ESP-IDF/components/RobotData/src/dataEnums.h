#ifndef DATA_ENUMS_H
#define DATA_ENUMS_H

struct MapData{
    int32_t MapTime;
    int32_t MapEncMedia;
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
    CMDTXT,

};

#endif