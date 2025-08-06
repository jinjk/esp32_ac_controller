#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Configuration
extern const char* ssid;
extern const char* password;

// ESP32-S3 Pin Definitions
// Note: IR_RECV_PIN removed - no IR receiver required for Gree AC
#define IR_SEND_PIN 13      // GPIO13 - IR Transmitter (Gree AC control)
#define WIND_SENSOR_PIN 12  // GPIO12 - Wind sensor
#define OLED_SDA 21         // GPIO21 - SDA for OLED
#define OLED_SCL 22         // GPIO22 - SCL for OLED
// GPIO14 now available for other uses (was IR_RECV_PIN)

// Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Time Configuration
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// AC Settings Structure
struct ACSetting {
  int startHour;
  int endHour;
  float temp;
  int wind;
};

// Global Variables
extern float currentTemp;
extern bool acOn;
extern ACSetting daySetting;
extern ACSetting nightSetting;

#endif
