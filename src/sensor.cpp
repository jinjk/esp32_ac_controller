#include "sensor.h"
#include "SHTSensor.h"
#include <Wire.h>

// Global sensor objects - auto-detect SHT sensor type
SHTSensor sht;

void initSensors() {
  // Initialize I2C with the pins defined in config.h
  Wire.begin(OLED_SDA, OLED_SCL);
  
  Serial.println("Scanning I2C bus for devices...");
  int deviceCount = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found! Check wiring:");
    Serial.println("SDA -> GPIO8, SCL -> GPIO9");
    return;
  }
  
  // Initialize SHT sensor with auto-detection
  if (sht.init()) {
    Serial.println("SHT sensor initialized successfully (auto-detected)");
    
    // Test reading
    delay(100);
    if (sht.readSample()) {
      float testTemp = sht.getTemperature();
      float testHum = sht.getHumidity();
      Serial.printf("Test reading - Temperature: %.2f°C, Humidity: %.1f%%\n", testTemp, testHum);
    } else {
      Serial.println("Failed to read test sample from sensor");
    }
  } else {
    Serial.println("Failed to initialize SHT sensor");
    Serial.println("Trying specific sensor types...");
    
    // Try SHT2x (SHT20/21/25) specifically
    sht = SHTSensor(SHTSensor::SHT2X);
    if (sht.init()) {
      Serial.println("SHT2x sensor (SHT20/21/25) initialized at address 0x40");
    } else {
      // Try SHT3x specifically  
      sht = SHTSensor(SHTSensor::SHT3X);
      if (sht.init()) {
        Serial.println("SHT3x sensor initialized");
      } else {
        Serial.println("No compatible SHT sensor found");
        Serial.println("Check connections and sensor model");
      }
    }
  }
}

float readTemperature() {
  if (!sht.readSample()) {
    Serial.println("Failed to read sample from SHT sensor");
    return NAN;
  }
  
  float temp = sht.getTemperature();
  
  // Check if temperature reading is valid
  if (isnan(temp)) {
    Serial.println("Failed to read temperature from SHT sensor - NaN");
    return NAN;
  }
  
  // Check if temperature is in reasonable range
  if (temp < -40 || temp > 80) {
    Serial.printf("Invalid temperature reading: %.2f°C (out of range)\n", temp);
    return NAN;
  }
  
  return temp;
}

float readHumidity() {
  if (!sht.readSample()) {
    Serial.println("Failed to read sample from SHT sensor");
    return NAN;
  }
  
  float humidity = sht.getHumidity();
  
  // Check if humidity reading is valid
  if (isnan(humidity)) {
    Serial.println("Failed to read humidity from SHT sensor - NaN");
    return NAN;
  }
  
  // Check if humidity is in reasonable range
  if (humidity < 0 || humidity > 100) {
    Serial.printf("Invalid humidity reading: %.1f%% (out of range)\n", humidity);
    return NAN;
  }
  
  return humidity;
}
