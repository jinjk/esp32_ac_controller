#include "config.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// WiFi Configuration
const char* ssid = "TP-LINK_0B75";
const char* password = "bafe@123";

// Time Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 8;  // GMT+8
const int daylightOffset_sec = 0;

// Debug mode flag - when true, always send IR commands regardless of state change
bool debugMode = false;

// Global Variables
float currentTemp = 0.0;

// Rule-based control system
ACRule rules[MAX_RULES];
int ruleCount = 0;
int activeRuleId = -1;

// Mutex for thread-safe access to rules
SemaphoreHandle_t rulesMutex = NULL;

// System timing configuration (in milliseconds)
uint32_t AC_CONTROL_LOOP_INTERVAL_MS = 5000;  // 60 seconds for AC control loop
uint32_t DISPLAY_REFRESH_INTERVAL_MS = 5000;   // 5 seconds for display refresh

// Initialize the rules mutex
void initRulesMutex() {
  if (rulesMutex == NULL) {
    rulesMutex = xSemaphoreCreateMutex();
    if (rulesMutex == NULL) {
      Serial.println("❌ Failed to create rules mutex!");
    } else {
      Serial.println("✅ Rules mutex created successfully");
    }
  }
}

// Initialize default rules
void initDefaultRules() {
  // Rule 1: Cool during hot days
  rules[0] = {
    .id = 1,
    .name = "Cool Day",
    .enabled = true,
    .startHour = 8,
    .endHour = 19,
    .minTemp = 26.0,
    .maxTemp = -999,
    .acOn = true,
    .setTemp = 27.0,
    .fanSpeed = 3,
    .mode = 0,
    .vSwing = 0,  // Auto vertical swing
    .hSwing = 0   // Auto horizontal swing
  };
  
  // Rule 2: Quiet cooling at night
  rules[1] = {
    .id = 2,
    .name = "Cool Night",
    .enabled = true,
    .startHour = 19,
    .endHour = 8,
    .minTemp = 26.0,
    .maxTemp = -999,
    .acOn = true,
    .setTemp = 28.0,
    .fanSpeed = 1,
    .mode = 0,
    .vSwing = 2,  // Mid vertical swing (less air movement for sleep)
    .hSwing = 2   // Mid horizontal swing
  };
  
  // Rule 3: Turn off when cool
  rules[2] = {
    .id = 3,
    .name = "Turn Off When Cool",
    .enabled = true,
    .startHour = -1,
    .endHour = -1,
    .minTemp = -999,
    .maxTemp = 25.9,
    .acOn = false,
    .setTemp = 24.0,
    .fanSpeed = 1,
    .mode = 0,
    .vSwing = 0,  // Auto (doesn't matter when AC is off)
    .hSwing = 0   // Auto (doesn't matter when AC is off)
  };
  
  ruleCount = 3;
}

// Sort rules by start time and then by minimum temperature
void sortRules() {
  if (ruleCount <= 1) return; // No need to sort if 0 or 1 rule
  
  // Simple bubble sort - sufficient for small arrays (max 10 rules)
  for (int i = 0; i < ruleCount - 1; i++) {
    for (int j = 0; j < ruleCount - i - 1; j++) {
      bool shouldSwap = false;
      
      // Compare start times first
      int startA = rules[j].startHour == -1 ? 999 : rules[j].startHour;     // -1 (any time) goes to end
      int startB = rules[j+1].startHour == -1 ? 999 : rules[j+1].startHour; // -1 (any time) goes to end
      
      if (startA > startB) {
        shouldSwap = true;
      } else if (startA == startB) {
        // If start times are equal, compare minimum temperatures
        float minTempA = rules[j].minTemp == -999 ? 999.0 : rules[j].minTemp;     // -999 (any temp) goes to end
        float minTempB = rules[j+1].minTemp == -999 ? 999.0 : rules[j+1].minTemp; // -999 (any temp) goes to end
        
        if (minTempA > minTempB) {
          shouldSwap = true;
        }
      }
      
      if (shouldSwap) {
        // Swap rules[j] and rules[j+1]
        ACRule temp = rules[j];
        rules[j] = rules[j+1];
        rules[j+1] = temp;
      }
    }
  }
}

// Save rules to SPIFFS
void saveRulesToSPIFFS() {
  // Acquire mutex for thread-safe access
  if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    // Sort rules before saving
    sortRules();
    JsonDocument doc;
    JsonArray rulesArray = doc["rules"].to<JsonArray>();
    
    for (int i = 0; i < ruleCount; i++) {
      JsonObject rule = rulesArray.add<JsonObject>();
      rule["id"] = rules[i].id;
      rule["name"] = rules[i].name;
      rule["enabled"] = rules[i].enabled;
      rule["startHour"] = rules[i].startHour;
      rule["endHour"] = rules[i].endHour;
      rule["minTemp"] = rules[i].minTemp;
      rule["maxTemp"] = rules[i].maxTemp;
      rule["acOn"] = rules[i].acOn;
      rule["setTemp"] = rules[i].setTemp;
      rule["fanSpeed"] = rules[i].fanSpeed;
      rule["mode"] = rules[i].mode;
      rule["vSwing"] = rules[i].vSwing;
      rule["hSwing"] = rules[i].hSwing;
    }
    
    doc["count"] = ruleCount;
    doc["version"] = 1; // For future migration compatibility
    
    // Release mutex before file I/O to minimize lock time
    xSemaphoreGive(rulesMutex);
    
    File file = SPIFFS.open("/rules.json", "w");
    if (file) {
      serializeJson(doc, file);
      file.close();
      Serial.printf("✅ Saved %d rules to SPIFFS\n", ruleCount);
    } else {
      Serial.println("❌ Failed to save rules to SPIFFS");
    }
  } else {
    Serial.println("⚠️ Failed to acquire rules mutex for saving");
  }
}

// Load rules from SPIFFS
void loadRulesFromSPIFFS() {
  File file = SPIFFS.open("/rules.json", "r");
  if (!file) {
    Serial.println("📄 No saved rules found, creating defaults");
    // Acquire mutex for thread-safe access
    if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      initDefaultRules();
      xSemaphoreGive(rulesMutex);
      saveRulesToSPIFFS(); // Save defaults for next time
    } else {
      Serial.println("⚠️ Failed to acquire rules mutex for default initialization");
    }
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("❌ Failed to parse rules.json: %s\n", error.c_str());
    Serial.println("📄 Using default rules instead");
    // Acquire mutex for thread-safe access
    if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      initDefaultRules();
      xSemaphoreGive(rulesMutex);
      saveRulesToSPIFFS(); // Overwrite corrupted file
    } else {
      Serial.println("⚠️ Failed to acquire rules mutex for default initialization");
    }
    return;
  }
  
  // Acquire mutex for thread-safe access
  if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    // Load rules from JSON
    JsonArray rulesArray = doc["rules"];
    ruleCount = 0;
    
    for (JsonObject rule : rulesArray) {
      if (ruleCount >= MAX_RULES) {
        Serial.printf("⚠️ Maximum rules (%d) reached, skipping remaining\n", MAX_RULES);
        break;
      }
      
      rules[ruleCount].id = rule["id"] | (ruleCount + 1); // Fallback ID
      rules[ruleCount].name = rule["name"] | String("Rule " + String(ruleCount + 1));
      rules[ruleCount].enabled = rule["enabled"] | true;
      rules[ruleCount].startHour = rule["startHour"] | -1;
      rules[ruleCount].endHour = rule["endHour"] | -1;
      rules[ruleCount].minTemp = rule["minTemp"] | -999.0f;
      rules[ruleCount].maxTemp = rule["maxTemp"] | -999.0f;
      rules[ruleCount].acOn = rule["acOn"] | true;
      rules[ruleCount].setTemp = rule["setTemp"] | 25.0f;
      rules[ruleCount].fanSpeed = rule["fanSpeed"] | 2;
      rules[ruleCount].mode = rule["mode"] | 0;
      rules[ruleCount].vSwing = rule["vSwing"] | 0;
      rules[ruleCount].hSwing = rule["hSwing"] | 0;
      
      ruleCount++;
    }
    
    // Release mutex
    xSemaphoreGive(rulesMutex);
    
    Serial.printf("✅ Loaded %d rules from SPIFFS\n", ruleCount);
    
    // If no rules were loaded, create defaults
    if (ruleCount == 0) {
      Serial.println("📄 No valid rules loaded, creating defaults");
      if (xSemaphoreTake(rulesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        initDefaultRules();
        xSemaphoreGive(rulesMutex);
        saveRulesToSPIFFS();
      }
    }
  } else {
    Serial.println("⚠️ Failed to acquire rules mutex for loading");
  }
}
