#ifndef DATA_ENUMS_H
#define DATA_ENUMS_H

enum CarState
{
    CAR_IN_CURVE,
    CAR_IN_LINE,
    CAR_STOPPED,
    CAR_TUNING,
};

enum CarSensor
{
    CAR_SENSOR_LEFT,
    CAR_SENSOR_RIGHT,
    CAR_SENSOR_FRONT
};

enum DataFunction
{
    RETORNO_OK,
    RETORNO_ERRO,
    RETORNO_ERRO_GENERICO,
    RETORNO_ARGUMENTO_INVALIDO,
    RETORNO_VARIAVEL_OCUPADA
};

enum PacketType
{
    PACKET_TYPE_CMD,
    PACKET_TYPE_RETURN
};

enum TrackState
{
    LONG_LINE,
    MEDIUM_LINE,
    SHORT_LINE,
    LONG_CURVE,
    MEDIUM_CURVE,
    SHORT_CURVE,
    ZIGZAG,
    SPECIAL_TRACK,
    DEFAULT_TRACK,
    UNDEFINED,     
    TUNING,
};

struct PacketData
{
    uint8_t id;           // ID do pacote
    uint8_t type;         // Tipo do pacote
    uint8_t numActual;    // Numero atual do pacote
    uint8_t numToReceive; // Numero total de pacotes
    size_t size;         // Tamanho do dado
    uint8_t data[240];    // Dados do pacote
};

#endif