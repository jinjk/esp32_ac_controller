#include "ac_control.h"
#include "sensor.h"
#include "ir_control.h"
#include <IRremoteESP8266.h>
#include <ir_Gree.h>
#include <time.h>

// Sleep interval for control loop (milliseconds)
static const uint32_t CONTROL_LOOP_SLEEP_MS = 10 * 1000;

// Track previous AC state to avoid unnecessary commands
static ACState previousACState = {false, 24, 0, 0, 0, 0};

// Helper function to check if AC state has changed
bool hasACStateChanged(bool power, uint8_t temp, uint8_t fan, uint8_t mode, int vSwing, int hSwing) {
  return (previousACState.power != power ||
          previousACState.temperature != temp ||
          previousACState.fanSpeed != fan ||
          previousACState.mode != mode ||
          previousACState.vSwing != vSwing ||
          previousACState.hSwing != hSwing);
}

// Helper function to update previous AC state
void updatePreviousACState(bool power, uint8_t temp, uint8_t fan, uint8_t mode, int vSwing, int hSwing) {
  previousACState.power = power;
  previousACState.temperature = temp;
  previousACState.fanSpeed = fan;
  previousACState.mode = mode;
  previousACState.vSwing = vSwing;
  previousACState.hSwing = hSwing;
}

// Function to get current AC state
ACState getCurrentACState() {
  return previousACState;
}

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
      vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_SLEEP_MS)); // 60 seconds - power efficient delay
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
        
        Serial.printf("Rule %d matches: %s (Temp: %.1f°C, Time: %02d:00)\n", 
                     rules[i].id, rules[i].name.c_str(), currentTemp, hour);
        
        // Check if AC state needs to change
        bool stateChanged = hasACStateChanged(rules[i].acOn, (uint8_t)rules[i].setTemp, 
                                            rules[i].fanSpeed, rules[i].mode, 
                                            rules[i].vSwing, rules[i].hSwing);
        
        if (stateChanged) {
          Serial.printf("AC State Change Detected - Applying Rule %d\n", rules[i].id);
          
          if (rules[i].acOn) {
            // Configure all AC settings first
            greeAC.powerOn();
            greeAC.setTemperature((uint8_t)rules[i].setTemp);
            greeAC.setFanSpeed(rules[i].fanSpeed);
            greeAC.setMode(rules[i].mode);
            greeAC.setSwingVPosition(rules[i].vSwing);
            greeAC.setSwingHPosition(rules[i].hSwing);
            
            // Send all settings at once
            greeAC.sendAllSettings();
            
            Serial.printf("AC ON: %.1f°C, Fan %d, Mode %d, VSwing %d, HSwing %d\n", 
                         rules[i].setTemp, rules[i].fanSpeed, rules[i].mode, 
                         rules[i].vSwing, rules[i].hSwing);
          } else {
            greeAC.powerOff();
            greeAC.sendAllSettings(); // Send the OFF command
            Serial.println("AC OFF");
          }
          
          // Update tracked state
          updatePreviousACState(rules[i].acOn, (uint8_t)rules[i].setTemp, 
                              rules[i].fanSpeed, rules[i].mode, 
                              rules[i].vSwing, rules[i].hSwing);
        } else {
          Serial.printf("AC State Unchanged - Rule %d already applied\n", rules[i].id);
        }
        
        break; // Stop at first match
      }
    }
    
    if (activeRuleId == -1) {
      Serial.println("No matching rules found");
      
      // Check if AC should be turned off (no rules match and AC was previously on)
      if (previousACState.power) {
        Serial.println("Turning AC OFF - No active rules");
        greeAC.powerOff();
        greeAC.sendAllSettings(); // Send the OFF command
        updatePreviousACState(false, 24, 0, 0, 0, 0); // Reset to default off state
      } else {
        Serial.println("AC already OFF - No change needed");
      }
    }

    // Log status
    logToCloud(currentTemp);
    
    vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_SLEEP_MS));
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
