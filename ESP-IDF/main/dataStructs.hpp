#include <stdint.h>
#include <stddef.h>

enum CarStatus
{
    IN_CURVE = 0,
    IN_LINE = 1
};

enum enum_espNOW
{
    // Request one time
    REQUEST_CARVALUES,
    REQUEST_CARPARAM,

    // Queue of send
    QUEUE_CARVALUES,
    QUEUE_CARPARAM,

    // Tipos de envio
    TYPE_CARVALUES,
    TYPE_CARPARAM
};

enum enum_parametros
{
    CURVA,
    RETA,
};

enum enum_paramsValidosCar
{
    NONE_VALID = 0b00000000,
    PID_VALID = 0b00000001,
    SPEED_VALID = 0b00000010,
    SARRAY_VALID = 0b00000100,
    SLAT_VALID = 0b00001000
};

struct valuesSArray
{
    uint16_t channel[8];
    int16_t line;
};

struct valuesSLat
{
    uint16_t channel[2];
    int16_t line;
};

struct valuesEnc
{
    int32_t encDir;
    int32_t encEsq;

    valuesEnc() : encDir(0), encEsq(0){};
};

struct valuesMarks
{
    int16_t leftPassed;
    int16_t rightPassed;
    bool sLatEsq;
    bool sLatDir;

    valuesMarks() : leftPassed(0), rightPassed(0){};
};

struct valuesPID
{
    // Parâmetros
    int16_t *input;
    float output;

    valuesPID() : input(NULL), output(0){};
};

struct valuesSpeed
{
    int8_t right;
    int8_t left;

    valuesSpeed() : right(0), left(0){};
};

struct paramSArray
{
    uint16_t maxChannel[8];
    uint16_t minChannel[8];
};

struct paramSLat
{
    uint16_t maxChannel[2];
    uint16_t minChannel[2];
};

struct paramSpeedVals
{
    uint8_t max;
    uint8_t min;
    uint8_t base;

    paramSpeedVals() : max(80), min(5), base(40){};
};

struct paramSpeed
{
    paramSpeedVals curva;
    paramSpeedVals reta;
};

struct paramPIDVals
{
    int16_t setpoint;
    float Kp;
    float Ki;
    float Kd;

    paramPIDVals() : setpoint(3500), Kp(0.01), Ki(0.00), Kd(0.10){};
};


struct paramPID
{
    paramPIDVals curva;
    paramPIDVals reta;
};

struct valuesCar
{
    int8_t state; // 0: parado, 1: linha, 2: curva
    valuesSpeed speed;
    valuesPID PID;
    valuesMarks latMarks;
    valuesEnc motEncs;
    valuesSLat sLat;
    valuesSArray sArray;
    uint32_t lastUpdate;

    valuesCar() : state(1), lastUpdate(0){};
};

struct paramsCar
{
    paramSpeed speed;
    paramPID PID;
    paramSLat sLat;
    paramSArray sArray;
    uint8_t validParams;

    paramsCar() : validParams(PID_VALID | SPEED_VALID){};
};

struct dataCar
{
    valuesCar values;
    paramsCar params;
};

struct valuesSamples
{
    valuesCar carVal;
    uint32_t time;

    valuesSamples(valuesCar carVal, uint32_t time) : carVal(carVal), time(time){};
    valuesSamples(){};
};

struct paramsSamples
{
    paramsCar carParam;
    uint32_t time;

    paramsSamples(paramsCar carParam, uint32_t time) : carParam(carParam), time(time){};
    paramsSamples(){};
};
