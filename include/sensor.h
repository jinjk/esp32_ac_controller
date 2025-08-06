#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"

// Forward declarations
class Adafruit_SHT31;

// Sensor management functions
void initSensors();
float readTemperature();
int readWindSensor();

// Global sensor objects
extern Adafruit_SHT31 sht31;

#endif
