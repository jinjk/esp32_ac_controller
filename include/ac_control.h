#ifndef AC_CONTROL_H
#define AC_CONTROL_H

#include "config.h"

// AC control functions
void initTime();
void controlTask(void* param);
void logToCloud(float temp);

// AC state functions
struct ACState {
  bool power;
  uint8_t temperature;
  uint8_t fanSpeed;
  uint8_t mode;
  int vSwing;
  int hSwing;
};

ACState getCurrentACState();

#endif
