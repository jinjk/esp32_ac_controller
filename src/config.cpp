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
