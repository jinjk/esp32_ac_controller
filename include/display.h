#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "config.h"

// Display management functions
void initDisplay();
void updateDisplay();
void displayTask(void* param);

// Global display object
extern Adafruit_SSD1306 display;

#endif
