#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "TrackSegment.hpp"
#include "driver/gpio.h"

#include "MappingService.hpp"
#include "LEDsService.hpp"
#include "SpeedService.hpp"


using namespace cpp_freertos;
#include "esp_log.h"

#define ManualMap

class CarStatusService : public Thread, public Singleton<CarStatusService>
{
public:
    
    CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;
    static SemaphoreHandle_t SemaphoreButton;


private:

    Robot *robot;
    RobotStatus *status;
    dataSpeed *speed;
    dataSLatMarks *latMarks;
    dataPID *PidTrans;

    CarState actualCarState, initialRobotState;

    TrackSegment TrackLen = SHORT_CURVE;

    MappingService *mappingService;

    int numMarks = 0; // Número total de marcações laterais na pista
    
    int iloop = 0;
    
    bool stateChanged; // verifica se o carrinho mudou seu estado quanto ao mapeamento
    bool lastTransition = false;
    bool transition = false;

    TrackSegment lastTrack = SHORT_LINE; // armazena último tipo de trecho da pista percorrido
    uint8_t lastState; // armazena último estado do mapeamento
    bool lastPaused = false;
    bool lastMappingState;
    int lastMarkPassed = 0;

    int16_t offsetnxt = 0;
    bool started_in_Tuning = false;
    int32_t mediaEncActual = 0;
    int32_t mediaEncFinal = 0;
    int32_t initialmediaEnc = 0;
    int32_t pulsesBeforeCurve = 200;
    int32_t pulsesAfterCurve = 200;
    bool firstmark = false;

    static void IRAM_ATTR gpio_isr_handler(void *arg);
    void configExternInterrupt(gpio_num_t gpio_num);
};

#endif