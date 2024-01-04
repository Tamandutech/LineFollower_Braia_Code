#ifndef TRACK_SEGMENT_HPP
#define TRACK_SEGMENT_HPP

#include <map>
#include <list>
#include <algorithm>

#include "dataSpeed.h"

enum TrackSegment
{
    SHORT_LINE = 2,
    MEDIUM_LINE = 1,
    LONG_LINE = 0,
    XLONG_LINE = 11,

    SHORT_CURVE = 5,
    MEDIUM_CURVE = 4,
    LONG_CURVE = 3,
    XLONG_CURVE = 12,

    ZIGZAG_TRACK = 6,
    SPECIAL_TRACK = 7,
    DEFAULT_TRACK = 13
};

float getTrackSegmentSpeed(TrackSegment trackSegment, dataSpeed *speed);
float getTargetSpeed(TrackSegment trackSegment, CarState estado, dataSpeed *speed);
std::map<TrackSegment, float> getSpeedValuesFromDashboard(dataSpeed *speed);
bool existTrackSegmentInList(std::list<TrackSegment> list, TrackSegment trackSegment);
bool LineSegment(TrackSegment trackSegment);
#endif


