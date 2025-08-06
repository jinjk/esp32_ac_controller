#include <IRremoteESP8266.h>
#include <ir_Gree.h>
#include "ac_control.h"
#include "sensor.h"
#include "ir_control.h"

void initTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized");
}

void controlTask(void* param) {
  Serial.println("AC Control Task started on Core " + String(xPortGetCoreID()));
  
  for (;;) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;

    // Read temperature
    currentTemp = readTemperature();
    
    // Check if temperature reading is valid
    if (isnan(currentTemp)) {
      Serial.println("Failed to read temperature from SHT31");
      vTaskDelay(pdMS_TO_TICKS(60000)); // 60 seconds - power efficient delay
      continue;
    }
    
    Serial.printf("Current Temperature: %.2f°C\n", currentTemp);

    // Get active settings based on time
    ACSetting activeSetting = getCurrentSettings();

    // AC Control Logic
    if (currentTemp < 26.0) {
      if (acOn) {
        Serial.println("Temperature low. Turning off AC.");
        acOn = false;
        // Could send specific OFF code here if different from main code
      }
    } else {
      if (!acOn) {
        Serial.printf("Temperature high (%.1f°C). Turning on AC with %s settings\n", 
                     currentTemp, (hour >= daySetting.startHour && hour < daySetting.endHour) ? "day" : "night");
        acOn = true;
        applyACSetting(activeSetting);
      }
    }

    // Log status
    logToCloud(currentTemp, acOn);
    
    // Wait 60 seconds before next check - allows core to sleep for power efficiency
    vTaskDelay(pdMS_TO_TICKS(60000));
  }
}

ACSetting getCurrentSettings() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  
  // Return day settings if current hour is between start and end hour
  if (hour >= daySetting.startHour && hour < daySetting.endHour) {
    return daySetting;
  } else {
    return nightSetting;
  }
}

void logToCloud(float temp, bool acStatus) {
  // Enhanced logging with timestamp
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  Serial.printf("[%02d:%02d:%02d] IoT Log - Temp: %.1f°C, AC: %s\n", 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
                temp, acStatus ? "ON" : "OFF");
  
  // Here you could add actual cloud logging (MQTT, HTTP POST, etc.)
  // Example: Send to ThingSpeak, AWS IoT, or similar service
}
