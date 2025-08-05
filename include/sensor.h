#ifndef SENSOR_H
#define SENSOR_H

#include <Adafruit_SHT31.h>
#include "config.h"

// Sensor management functions
void initSensors();
float readTemperature();
int readWindSensor();

// Global sensor objects
extern Adafruit_SHT31 sht31;

#endif
