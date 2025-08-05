#include "display.h"

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
  display.printf("Temp: %.1fC\n", currentTemp);
  display.printf("AC: %s\n", acOn ? "ON" : "OFF");
  display.printf("Mode: %s\n", (timeinfo->tm_hour >= daySetting.startHour && timeinfo->tm_hour < daySetting.endHour) ? "Day" : "Night");
  display.display();
}

void displayTask(void* param) {
  Serial.println("Display Task started on Core " + String(xPortGetCoreID()));
  
  for (;;) {
    updateDisplay();
    
    // Use vTaskDelay for power efficiency - allows core to sleep
    // Refresh every 5 seconds - good balance between responsiveness and power
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
