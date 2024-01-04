#ifndef LED_STATUS_HPP
#define LED_STATUS_HPP

#include <map>
#include <list>
#include <algorithm>
#include "TrackSegment.hpp"

enum led_color_t
{
    // Red, Green, Blue
    LED_COLOR_BLACK = 0x000000,
    LED_COLOR_RED = 0xFF0000,
    LED_COLOR_GREEN = 0x00FF00,
    LED_COLOR_BLUE = 0x0000FF,
    LED_COLOR_YELLOW = 0xFFFF00,
    LED_COLOR_CYAN = 0x00FFFF,
    LED_COLOR_MAGENTA = 0xFF00FF,
    LED_COLOR_WHITE = 0xFFFFFF,
    LED_COLOR_PURPLE = 0x7F007F,
    LED_COLOR_ORANGE = 0xFF7F00,
    LED_COLOR_BROWN = 0x7F3F00,
    LED_COLOR_LIME = 0x3FFF00,
    LED_COLOR_PINK = 0xFF007F,
    LED_COLOR_TURQUOISE = 0x00FF7F,
    LED_COLOR_VIOLET = 0x7F00FF,
};

static std::map<TrackSegment, float> segmentBrightness{
    {TrackSegment::SHORT_LINE, 0.05},
    {TrackSegment::MEDIUM_LINE, 0.3},
    {TrackSegment::LONG_LINE, 1.0},
    {TrackSegment::XLONG_LINE, 1.0},

    {TrackSegment::SHORT_CURVE, 0.05},
    {TrackSegment::MEDIUM_CURVE, 0.3},
    {TrackSegment::LONG_CURVE, 1.0},
    {TrackSegment::XLONG_CURVE, 1.0},

    {TrackSegment::ZIGZAG_TRACK, 1},
    {TrackSegment::SPECIAL_TRACK, 0.05}

};

static std::map<CarState, led_color_t> statusColor{
    {CAR_TUNING, LED_COLOR_WHITE},
    {CAR_MAPPING, LED_COLOR_YELLOW},
    {CAR_ENC_READING_BEFORE_FIRSTMARK, LED_COLOR_PURPLE},
};


led_color_t getStatusColor(CarState estado, TrackSegment segment);
float getSegmentBrightness(CarState estado, TrackSegment segment);

#endif