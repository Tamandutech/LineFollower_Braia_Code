#ifndef MAIN_CPP_H
#define MAIN_CPP_H

#include "platform_functions.h"

extern pinhandler_t bootSw;

extern pinhandler_t leds;

extern pinhandler_t motorDirDir;
extern pwmhandler_t motorDirPWM;
extern pinhandler_t motorEsqDir;
extern pwmhandler_t motorEsqPWM;

extern pwmhandler_t motorSucPWM;

int loop_old(void);

#endif  // MAIN_CPP_H
