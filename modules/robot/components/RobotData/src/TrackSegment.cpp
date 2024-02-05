#include "TrackSegment.hpp"

std::map<TrackSegment, float> getSpeedValuesFromDashboard(dataSpeed *speed)
{
    std::map<TrackSegment, float> TrackSegmentSpeed{
        {TrackSegment::SHORT_LINE, speed->Short_Line->getData()},
        {TrackSegment::MEDIUM_LINE, speed->Medium_Line->getData()},
        {TrackSegment::LONG_LINE, speed->Long_Line->getData()},
        {TrackSegment::XLONG_LINE, speed->XLong_Line->getData()},

        {TrackSegment::SHORT_CURVE, speed->Short_Curve->getData()},
        {TrackSegment::MEDIUM_CURVE, speed->Medium_Curve->getData()},
        {TrackSegment::LONG_CURVE, speed->Long_Curve->getData()},
        {TrackSegment::XLONG_CURVE, speed->XLong_Curve->getData()},

        {TrackSegment::ZIGZAG_TRACK, speed->ZIGZAG->getData()},
        {TrackSegment::SPECIAL_TRACK, speed->Special_Track->getData()}
    };
    return TrackSegmentSpeed;
}

float getTrackSegmentSpeed(TrackSegment trackSegment, dataSpeed *speed)
{
    std::map<TrackSegment, float> TrackSegmentSpeed = getSpeedValuesFromDashboard(speed);
    bool trackStatusFound = TrackSegmentSpeed.find(trackSegment) != TrackSegmentSpeed.end();

    if (trackStatusFound)
        return TrackSegmentSpeed[trackSegment];

    return speed->Default_speed->getData();
}

// TODO: Refatorar essa função
float getTargetSpeed(TrackSegment trackSegment, CarState estado, dataSpeed *speed)
{
    float speedTarget = speed->Default_speed->getData();
    if (estado == CAR_ENC_READING_BEFORE_FIRSTMARK)
    {
        speedTarget = speed->initialspeed->getData();
    }
    else if (estado == CAR_ENC_READING)
    {
        speedTarget = getTrackSegmentSpeed(trackSegment, speed);
    }
    else if (estado == CAR_TUNING)
    {
        speedTarget = speed->Tunning_speed->getData();
    }
    return speedTarget;
}

bool existTrackSegmentInList(std::list<TrackSegment> list, TrackSegment trackSegment) {
    return (std::find(list.begin(), list.end(), trackSegment) != list.end());
}

bool isLineSegment(TrackSegment trackSegment)
{
    const std::list<TrackSegment> line = {SHORT_LINE, MEDIUM_LINE, LONG_LINE, XLONG_LINE};
    return existTrackSegmentInList(line, trackSegment);
}
bool isCurveSegment(TrackSegment trackSegment)
{
    return !isLineSegment(trackSegment);
}