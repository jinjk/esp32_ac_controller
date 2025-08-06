#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

// Forward declarations
class Adafruit_SSD1306;

// Display management functions
void initDisplay();
void updateDisplay();
void displayTask(void* param);

// Global display object
extern Adafruit_SSD1306 display;

#endif
