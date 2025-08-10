#include "ac_control.h"
#include "sensor.h"
#include "ir_control.h"
#include <IRremoteESP8266.h>
#include <ir_Gree.h>
#include <time.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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

// Function to safely copy rules for thread-safe access
// Returns the number of rules copied, or -1 if mutex acquisition failed
int copyRulesThreadSafe(ACRule localRules[], int maxRules) {
  int localRuleCount = 0;
  
  // Acquire mutex to safely copy rules
  if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Copy rules to local array
    for (int i = 0; i < ruleCount && i < maxRules; i++) {
      localRules[i] = rules[i];
    }
    localRuleCount = ruleCount;
    xSemaphoreGive(rulesMutex);
    return localRuleCount;
  } else {
    Serial.println("⚠️ Failed to acquire rules mutex for reading");
    return -1;
  }
}

void initTime() {
  Serial.println("Initializing time synchronization...");
  
  // Wait for WiFi to be fully connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi not connected, waiting...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n❌ WiFi connection failed - time sync will not work");
      return;
    }
    Serial.println("\n✅ WiFi connected");
  }
  
  // Configure NTP with multiple servers for reliability
  configTime(gmtOffset_sec, daylightOffset_sec, 
            "pool.ntp.org", 
            "time.nist.gov", 
            "time.cloudflare.com");
  
  Serial.println("📡 Requesting time from NTP servers...");
  
  // Wait for time synchronization (up to 30 seconds)
  struct tm timeinfo;
  int attempts = 0;
  bool timeSet = false;
  
  while (attempts < 30 && !timeSet) {
    if (getLocalTime(&timeinfo)) {
      // Check if we got a reasonable time (after year 2020)
      if (timeinfo.tm_year > (2020 - 1900)) {
        timeSet = true;
        Serial.printf("✅ Time synchronized: %04d-%02d-%02d %02d:%02d:%02d (GMT+8)\n",
                     timeinfo.tm_year + 1900, 
                     timeinfo.tm_mon + 1, 
                     timeinfo.tm_mday,
                     timeinfo.tm_hour, 
                     timeinfo.tm_min, 
                     timeinfo.tm_sec);
        break;
      }
    }
    delay(1000);
    attempts++;
    if (attempts % 5 == 0) {
      Serial.printf("⏳ Still waiting for time sync... (%d/30)\n", attempts);
    }
  }
  
  if (!timeSet) {
    Serial.println("❌ Time synchronization failed - using default time");
    Serial.println("⚠️ Rule scheduling may not work correctly");
  }
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
      Serial.println("Failed to read temperature, waiting for valid reading");
      vTaskDelay(pdMS_TO_TICKS(AC_CONTROL_LOOP_INTERVAL_MS)); // Use global configuration - power efficient delay
      continue;
    }
    
    Serial.printf("Current Temperature: %.2f°C\n", currentTemp);

    // Rule-based AC Control Logic
    activeRuleId = -1; // Reset active rule
    
    // Create local copy of rules for thread-safe access
    ACRule localRules[MAX_RULES];
    int localRuleCount = copyRulesThreadSafe(localRules, MAX_RULES);
    
    // Check if rule copying was successful
    if (localRuleCount == -1) {
      Serial.println("⚠️ Skipping this cycle due to mutex acquisition failure");
      vTaskDelay(pdMS_TO_TICKS(AC_CONTROL_LOOP_INTERVAL_MS));
      continue;
    }
    
    // Find first matching rule using local copy
    for (int i = 0; i < localRuleCount; i++) {
      if (!localRules[i].enabled) continue;
      
      bool timeMatch = true;
      bool tempMatch = true;
      
      // Check time conditions
      if (localRules[i].startHour != -1 && localRules[i].endHour != -1) {
        if (localRules[i].endHour > localRules[i].startHour) {
          // Normal time range (e.g., 8-19)
          timeMatch = (hour >= localRules[i].startHour && hour < localRules[i].endHour);
        } else {
          // Overnight time range (e.g., 19-8)
          timeMatch = (hour >= localRules[i].startHour || hour < localRules[i].endHour);
        }
      }
      
      // Check temperature conditions
      if (localRules[i].minTemp != -999 && currentTemp < localRules[i].minTemp) {
        tempMatch = false;
      }
      if (localRules[i].maxTemp != -999 && currentTemp > localRules[i].maxTemp) {
        tempMatch = false;
      }
      
      // If both conditions match, apply this rule
      if (timeMatch && tempMatch) {
        activeRuleId = localRules[i].id;
        
        Serial.printf("Rule %d matches: %s (Temp: %.1f°C, Time: %02d:00)\n", 
                     localRules[i].id, localRules[i].name.c_str(), currentTemp, hour);
        
        // Check if AC state needs to change OR if debug mode is enabled
        bool stateChanged = hasACStateChanged(localRules[i].acOn, (uint8_t)localRules[i].setTemp, 
                                            localRules[i].fanSpeed, localRules[i].mode, 
                                            localRules[i].vSwing, localRules[i].hSwing);
        
        if (stateChanged || debugMode) {
          if (debugMode && !stateChanged) {
            Serial.printf("🔧 DEBUG MODE: Force sending IR command for Rule %d (no state change)\n", localRules[i].id);
          } else {
            Serial.printf("AC State Change Detected - Applying Rule %d\n", localRules[i].id);
          }
          
          if (localRules[i].acOn) {
            // Configure all AC settings first
            greeAC.powerOn();
            greeAC.setTemperature((uint8_t)localRules[i].setTemp);
            greeAC.setFanSpeed(localRules[i].fanSpeed);
            greeAC.setMode(localRules[i].mode);
            greeAC.setSwingVPosition(localRules[i].vSwing);
            greeAC.setSwingHPosition(localRules[i].hSwing);
            
            // Send all settings at once
            greeAC.sendAllSettings();
            
            Serial.printf("AC ON: %.1f°C, Fan %d, Mode %d, VSwing %d, HSwing %d %s\n", 
                         localRules[i].setTemp, localRules[i].fanSpeed, localRules[i].mode, 
                         localRules[i].vSwing, localRules[i].hSwing,
                         debugMode ? "[DEBUG]" : "");
          } else {
            greeAC.powerOff();
            greeAC.sendAllSettings(); // Send the OFF command
            Serial.printf("AC OFF %s\n", debugMode ? "[DEBUG]" : "");
          }
          
          // Update tracked state
          updatePreviousACState(localRules[i].acOn, (uint8_t)localRules[i].setTemp, 
                              localRules[i].fanSpeed, localRules[i].mode, 
                              localRules[i].vSwing, localRules[i].hSwing);
        } else {
          if (debugMode) {
            Serial.printf("🔧 DEBUG MODE: Force sending IR command for Rule %d (no state change)\n", localRules[i].id);
          } else {
            Serial.printf("AC State Unchanged - Rule %d already applied\n", localRules[i].id);
          }
        }
        
        break; // Stop at first match
      }
    }
    
    if (activeRuleId == -1) {
      Serial.println("No matching rules found");
      
      // Check if AC should be turned off (no rules match and AC was previously on)
      if (previousACState.power || debugMode) {
        if (debugMode && !previousACState.power) {
          Serial.println("🔧 DEBUG MODE: Force sending AC OFF command (already off)");
        } else {
          Serial.println("Turning AC OFF - No active rules");
        }
        greeAC.powerOff();
        greeAC.sendAllSettings(); // Send the OFF command
        updatePreviousACState(false, 24, 0, 0, 0, 0); // Reset to default off state
      } else {
        Serial.println("AC already OFF - No change needed");
      }
    }

    // Log status
    logToCloud(currentTemp);
    
    vTaskDelay(pdMS_TO_TICKS(AC_CONTROL_LOOP_INTERVAL_MS));
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
