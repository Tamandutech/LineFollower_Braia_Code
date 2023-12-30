#include "TrackSegment.hpp"

std::map<TrackSegment, float> CreateTrackSegmentDict(dataSpeed *speed)
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

    std::map<TrackSegment, float> TrackSegmentSpeed = CreateTrackSegmentDict(speed);

    bool trackStatusFound = TrackSegmentSpeed.find(trackSegment) != TrackSegmentSpeed.end();

    if (trackStatusFound)
        return TrackSegmentSpeed[trackSegment];

    return speed->Default_speed->getData();
}