#include "sensor.h"
#include <Adafruit_SHT31.h>

// Global sensor objects
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void initSensors() {
  if (!sht31.begin(0x44)) {
    Serial.println("Could not find SHT31 sensor");
  } else {
    Serial.println("SHT31 sensor initialized");
  }
}

float readTemperature() {
  float temp = sht31.readTemperature();
  
  // Check if temperature reading is valid
  if (isnan(temp)) {
    Serial.println("Failed to read temperature from SHT31");
    return NAN;
  }
  
  return temp;
}
