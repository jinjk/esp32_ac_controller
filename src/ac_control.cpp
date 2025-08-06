#include "ac_control.h"
#include "sensor.h"
#include "ir_control.h"
#include <IRremoteESP8266.h>
#include <ir_Gree.h>
#include <time.h>

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

    // Simple AC Control Logic - Always send appropriate commands
    if (currentTemp < 26.0) {
      // Temperature is low - turn off AC
      Serial.println("Temperature low (<26°C). Sending AC OFF command.");
      greeAC.powerOff();
    } else {
      // Temperature is high (>=26°C) - turn on AC with time-based settings
      bool isDayTime = (hour >= daySetting.startHour && hour < daySetting.endHour);
      
      if (isDayTime) {
        // Day time: 27°C, Wind 3
        Serial.printf("Day time: Setting AC to 27°C, Wind 3 (Current temp: %.1f°C)\n", currentTemp);
        greeAC.powerOn();
        greeAC.setTemperature(27);
        greeAC.setFanSpeed(3);
      } else {
        // Night time: 28°C, Wind 1  
        Serial.printf("Night time: Setting AC to 28°C, Wind 1 (Current temp: %.1f°C)\n", currentTemp);
        greeAC.powerOn();
        greeAC.setTemperature(28);
        greeAC.setFanSpeed(1);
      }
    }

    // Log status
    logToCloud(currentTemp);
    
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

void logToCloud(float temp) {
  // Enhanced logging with timestamp
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  Serial.printf("[%02d:%02d:%02d] IoT Log - Temp: %.1f°C\n", 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, temp);
  
  // Here you could add actual cloud logging (MQTT, HTTP POST, etc.)
  // Example: Send to ThingSpeak, AWS IoT, or similar service
}
