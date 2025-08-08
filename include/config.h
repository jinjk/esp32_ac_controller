#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Configuration
extern const char* ssid;
extern const char* password;

// ESP32-S3 Pin Definitions
// Note: IR_RECV_PIN removed - no IR receiver required for Gree AC
#define IR_SEND_PIN 13      // GPIO13 - IR Transmitter (Gree AC control)
#define OLED_SDA 8          // GPIO8 - SDA for OLED and SHT31 (I2C shared bus)
#define OLED_SCL 9          // GPIO9 - SCL for OLED and SHT31 (I2C shared bus)
// GPIO12, GPIO14 now available for other uses

// Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Time Configuration
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Rule-based AC Control Structure
struct ACRule {
  int id;                    // Unique rule ID
  String name;               // Rule name/description
  bool enabled;              // Rule active/inactive
  
  // Time conditions (optional - use -1 to ignore)
  int startHour;             // Start hour (0-23, -1 = any)
  int endHour;               // End hour (0-23, -1 = any)
  
  // Temperature conditions (optional - use -999 to ignore)
  float minTemp;             // Minimum temperature (-999 = any)
  float maxTemp;             // Maximum temperature (-999 = any)
  
  // AC Actions
  bool acOn;                 // Turn AC on/off
  float setTemp;             // Target temperature
  int fanSpeed;              // Fan speed (0-3)
  int mode;                  // AC mode (0=cool, 1=heat, 2=dry, 3=fan, 4=auto)
  int vSwing;                // Vertical swing (0=auto, 1=top, 2=mid, 3=bottom)
  int hSwing;                // Horizontal swing (0=auto, 1=left, 2=mid, 3=right)
};

#define MAX_RULES 10

// Global Variables
extern float currentTemp;
extern ACRule rules[MAX_RULES];
extern int ruleCount;
extern int activeRuleId;

// Function declarations
void initDefaultRules();
void saveRulesToSPIFFS();
void loadRulesFromSPIFFS();

#endif
