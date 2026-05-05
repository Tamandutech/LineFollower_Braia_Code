/*
 * GobalData.cpp
 *
 *  Created on: Nov 15, 2025
 *      Author: Kelvin Novais
 */

#include "GlobalData.hpp"

GlobalData globalData = {.action = None,
                         .isReadyToRun{false},
                         .finishLineCount{0},
                         .mapData{},
                         .markCount{0}};
