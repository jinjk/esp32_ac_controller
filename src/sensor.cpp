#include "sensor.h"

// Global sensor objects
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void initSensors() {
  if (!sht31.begin(0x44)) {
    Serial.println("Could not find SHT31 sensor");
  } else {
    Serial.println("SHT31 sensor initialized");
  }
  
  pinMode(WIND_SENSOR_PIN, INPUT);
  Serial.println("Wind sensor pin initialized");
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

int readWindSensor() {
  // Read analog value from wind sensor
  // This is a placeholder - implement based on your specific wind sensor
  int windValue = analogRead(WIND_SENSOR_PIN);
  
  // Convert to wind speed or level based on your sensor specifications
  // This is a simple mapping - adjust based on your sensor
  int windLevel = map(windValue, 0, 4095, 1, 5);
  
  return constrain(windLevel, 1, 5);
}
