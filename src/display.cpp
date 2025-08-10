#include "display.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>

// Global display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initDisplay() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("AC Controller Booting...");
  display.display();
  Serial.println("Display initialized");
}

void updateDisplay() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Temp: %.1f C\n", currentTemp);
  display.printf("Rule: %s\n", activeRuleId != -1 ? "Active" : "None");
  display.printf("Time: %02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Show IP address if connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    display.printf("WiFi: Disconnected\n");
  }
  
  display.display();
}

void displayTask(void* param) {
  Serial.println("Display Task started on Core " + String(xPortGetCoreID()));
  
  for (;;) {
    updateDisplay();
    
    // Use vTaskDelay for power efficiency - allows core to sleep
    // Use global configuration for display refresh timing
    vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_INTERVAL_MS));
  }
}
