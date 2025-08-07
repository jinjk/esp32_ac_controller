#include "config.h"
#include <Arduino.h>

// WiFi Configuration
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Time Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 8;  // GMT+8
const int daylightOffset_sec = 0;

// Global Variables
float currentTemp = 0.0;
ACSetting daySetting = {8, 19, 27.0, 3};
ACSetting nightSetting = {19, 8, 28.0, 2};

// Rule-based control system
ACRule rules[MAX_RULES];
int ruleCount = 0;
int activeRuleId = -1;

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
    .mode = 0
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
    .mode = 0
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
    .mode = 0
  };
  
  ruleCount = 3;
}
