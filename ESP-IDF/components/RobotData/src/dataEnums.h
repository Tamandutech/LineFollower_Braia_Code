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
    MarkData,

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