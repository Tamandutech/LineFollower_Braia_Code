#ifndef TRACK_SEGMENT__HPP
#define TRACK_SEGMENT__HPP

#include <map>
#include <list>
#include <algorithm>

#include "dataSpeed.h"

enum TrackSegment
{
    SHORT_LINE = 1,
    MEDIUM_LINE = 2,
    LONG_LINE = 3,
    XLONG_LINE = 4,

    SHORT_CURVE = 5,
    MEDIUM_CURVE = 6,
    LONG_CURVE = 7,
    XLONG_CURVE = 8,

    ZIGZAG_TRACK = 9,
    SPECIAL_TRACK = 10,
    DEFAULT_TRACK = 11
};

float getTrackSegmentSpeed(TrackSegment trackSegment, dataSpeed *speed);
std::map<TrackSegment, float> CreateTrackSegmentDict(dataSpeed *speed);
#endif


