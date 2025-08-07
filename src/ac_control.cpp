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

    // Rule-based AC Control Logic
    activeRuleId = -1; // Reset active rule
    
    // Find first matching rule
    for (int i = 0; i < ruleCount; i++) {
      if (!rules[i].enabled) continue;
      
      bool timeMatch = true;
      bool tempMatch = true;
      
      // Check time conditions
      if (rules[i].startHour != -1 && rules[i].endHour != -1) {
        if (rules[i].endHour > rules[i].startHour) {
          // Normal time range (e.g., 8-19)
          timeMatch = (hour >= rules[i].startHour && hour < rules[i].endHour);
        } else {
          // Overnight time range (e.g., 19-8)
          timeMatch = (hour >= rules[i].startHour || hour < rules[i].endHour);
        }
      }
      
      // Check temperature conditions
      if (rules[i].minTemp != -999 && currentTemp < rules[i].minTemp) {
        tempMatch = false;
      }
      if (rules[i].maxTemp != -999 && currentTemp > rules[i].maxTemp) {
        tempMatch = false;
      }
      
      // If both conditions match, apply this rule
      if (timeMatch && tempMatch) {
        activeRuleId = rules[i].id;
        
        Serial.printf("Applying Rule %d: %s (Temp: %.1f°C, Time: %02d:00)\n", 
                     rules[i].id, rules[i].name.c_str(), currentTemp, hour);
        
        if (rules[i].acOn) {
          greeAC.powerOn();
          greeAC.setTemperature((uint8_t)rules[i].setTemp);
          greeAC.setFanSpeed(rules[i].fanSpeed);
          greeAC.setMode(rules[i].mode);
          greeAC.setSwingVPosition(rules[i].vSwing);
          greeAC.setSwingHPosition(rules[i].hSwing);
          Serial.printf("AC ON: %.1f°C, Fan %d, Mode %d, VSwing %d, HSwing %d\n", 
                       rules[i].setTemp, rules[i].fanSpeed, rules[i].mode, 
                       rules[i].vSwing, rules[i].hSwing);
        } else {
          greeAC.powerOff();
          Serial.println("AC OFF");
        }
        
        break; // Stop at first match
      }
    }
    
    if (activeRuleId == -1) {
      Serial.println("No matching rules found - AC unchanged");
    }

    // Log status
    logToCloud(currentTemp);
    
    // Wait 60 seconds before next check - allows core to sleep for power efficiency
    vTaskDelay(pdMS_TO_TICKS(60000));
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
