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
    dataMapping *MappingData;
    dataPID *PidTrans;

    CarState initialRobotState, currentRobotState, lastRobotState;

    MappingService *mappingService;

    int numMarks = 0; // Número total de marcações laterais na pista
    
    int iloop = 0;
    
    bool transition, lastTransition = false;

    TrackSegment transitionTrackSegment;
    TrackSegment lastTrack = SHORT_LINE; // armazena último tipo de trecho da pista percorrido
    bool lastPaused = false;

    MapData finalMark;
    int32_t robotPosition = 0;
    int32_t pulsesBeforeCurve = 200;

    static void IRAM_ATTR startRobotWithBootButton(void *arg);
    void configExternInterruptToReadButton(gpio_num_t gpio_num);
    void startFollowingDefinedMapping();
    void defineIfRobotWillStartMappingMode();
    void waitPressBootButtonToStart();
    void deleteMappingIfBootButtonIsPressed();
    void startMappingTheTrack();
    void setTuningMode();
    bool passedFirstMark();
    void resetEnconderInFirstMark();
    bool trackSegmentChanged();
    bool RobotStateChanged();
    LedColor defineLedColor();
    void setColorBrightness(LedColor color);
    void logCarStatus();
    void stopTunningMode();
    void defineTrackSegment(MapData Mark);
    TrackSegment getTrackSegment(MapData Mark);
};

#endif