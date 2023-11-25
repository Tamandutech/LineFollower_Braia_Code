#include <map>

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
    XLONG_LINE,
    XLONG_CURVE,
};

std::map<TrackState, float> TrackStatus;

TrackStatus[LONG_LINE] = speed->XLong_Line->getData();


float mapStatusConvert(TrackState trackState) {
    
}
