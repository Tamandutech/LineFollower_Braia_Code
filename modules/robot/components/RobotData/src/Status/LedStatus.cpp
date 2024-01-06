#include "LedStatus.hpp"

LedColor getStatusColor(CarState estado, TrackSegment segment)
{
    
    LedColor color = LED_COLOR_BLACK;
    bool statusColorFound = statusColor.find(estado) != statusColor.end();
    if(statusColorFound)
        return statusColor[estado];

    if(estado == CAR_ENC_READING)
    {
        color = LED_COLOR_RED;
        if(isLineSegment(segment)) 
            color = LED_COLOR_GREEN;
    }

    return color;
}

float getSegmentBrightness(CarState estado, TrackSegment segment)
{
    float brightness = 1;
    bool segmentBrightnessFound = segmentBrightness.find(segment) != segmentBrightness.end();
    if(segmentBrightnessFound && estado == CAR_ENC_READING)
        return segmentBrightness[segment];

    return brightness;
}