#ifndef AC_CONTROL_H
#define AC_CONTROL_H

#include <time.h>
#include "config.h"

// AC control functions
void initTime();
void controlTask(void* param);
void logToCloud(float temp, bool acStatus);
ACSetting getCurrentSettings();

#endif
