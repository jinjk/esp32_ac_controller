#ifndef SENSOR_H
#define SENSOR_H

#include "config.h"

// Forward declarations
class SHTSensor;

// Sensor management functions
void initSensors();
float readTemperature();
float readHumidity();

// Global sensor objects
extern SHTSensor sht;

#endif
