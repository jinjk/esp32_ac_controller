#include "web_server.h"
#include "task_manager.h"
#include "ir_control.h"
#include <SPIFFS.h>

// Global web server object
AsyncWebServer server(80);

void initWiFi() {
  Serial.println("Starting ESP32-S3 AC Controller...");
  Serial.printf("ESP32-S3 Chip: %d cores, %d MHz\n", ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %d MB, PSRAM: %d MB\n", ESP.getFlashChipSize() / (1024*1024), ESP.getPsramSize() / (1024*1024));
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
}

void setupWebServer() {
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getWebContent());
  });

  server.on("/learn", HTTP_POST, handleIRLearn);
  server.on("/send_ir", HTTP_POST, handleIRSend);
  server.on("/set", HTTP_POST, handleSettingsUpdate);
  server.on("/task", HTTP_POST, handleTaskControl);
  server.on("/status", HTTP_GET, handleTaskStatus);
  server.on("/stop", HTTP_POST, handleStopTask);

  server.begin();
  Serial.println("Web server started");
}

String readFile(String path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file: " + path);
    return "";
  }
  
  String content = file.readString();
  file.close();
  return content;
}

String getWebContent() {
  // Read the HTML template from SPIFFS
  String html = readFile("/index.html");
  
  if (html.isEmpty()) {
    // Fallback if file reading fails
    return R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><title>ESP32-S3 AC Controller</title></head>
      <body>
        <h1>ESP32-S3 AC Controller</h1>
        <p>Error: Could not load web interface. Please check SPIFFS.</p>
        <p><a href="/status">View Status</a></p>
      </body>
      </html>
    )rawliteral";
  }
  
  // Inject dynamic data into the HTML
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  String mode = (timeinfo->tm_hour >= daySetting.startHour && timeinfo->tm_hour < daySetting.endHour) ? "Day" : "Night";
  
  // Add dynamic data injection via JavaScript
  String dynamicScript = R"rawliteral(
    <script>
      // Dynamic data from ESP32
      window.ESP32_DATA = {
        currentTemp: )rawliteral" + String(currentTemp, 1) + R"rawliteral(,
        acOn: )rawliteral" + (acOn ? "true" : "false") + R"rawliteral(,
        mode: ")rawliteral" + mode + R"rawliteral(",
        daySetting: {
          temp: )rawliteral" + String(daySetting.temp, 1) + R"rawliteral(,
          wind: )rawliteral" + String(daySetting.wind) + R"rawliteral(,
          startHour: )rawliteral" + String(daySetting.startHour) + R"rawliteral(,
          endHour: )rawliteral" + String(daySetting.endHour) + R"rawliteral(
        },
        nightSetting: {
          temp: )rawliteral" + String(nightSetting.temp, 1) + R"rawliteral(,
          wind: )rawliteral" + String(nightSetting.wind) + R"rawliteral(
        }
      };
      
      // Update page with dynamic data when loaded
      document.addEventListener('DOMContentLoaded', function() {
        // Update status display
        document.getElementById('temperature-status').textContent = '🌡️ Temperature: ' + window.ESP32_DATA.currentTemp + '°C';
        document.getElementById('ac-status').textContent = '❄️ AC Status: ' + (window.ESP32_DATA.acOn ? '🟢 ON' : '🔴 OFF');
        document.getElementById('mode-status').textContent = '🕐 Mode: ' + window.ESP32_DATA.mode;
        
        // Update form values
        document.getElementById('dayTemp').value = window.ESP32_DATA.daySetting.temp;
        document.getElementById('dayWind').value = window.ESP32_DATA.daySetting.wind;
        document.getElementById('nightTemp').value = window.ESP32_DATA.nightSetting.temp;
        document.getElementById('nightWind').value = window.ESP32_DATA.nightSetting.wind;
      });
    </script>
  )rawliteral";
  
  // Insert the dynamic script before closing </body> tag
  html.replace("</body>", dynamicScript + "</body>");
  
  return html;
}

void handleIRLearn(AsyncWebServerRequest *request) {
  // Start IR learning task dynamically
  if (taskManager.startIRLearningTask()) {
    request->send(200, "text/plain", "✅ IR learning task started. Point remote at device and press button.");
  } else {
    request->send(500, "text/plain", "❌ Failed to start IR learning task. Task may already be running.");
  }
}

void handleIRSend(AsyncWebServerRequest *request) {
  if (!request->hasParam("button", true)) {
    request->send(400, "text/plain", "Missing button parameter");
    return;
  }
  
  String buttonStr = request->getParam("button", true)->value();
  int buttonIndex = buttonStr.toInt();
  
  if (buttonIndex >= 0 && buttonIndex < 14) {
    IRButton button = (IRButton)buttonIndex;
    String code = getIRCodeForButton(button);
    
    if (code.length() > 0) {
      sendIRButton(button);
      request->send(200, "text/plain", "✅ Sent " + String(buttonNames[button]) + " command");
    } else {
      request->send(404, "text/plain", "❌ No IR code learned for " + String(buttonNames[button]));
    }
  } else {
    request->send(400, "text/plain", "❌ Invalid button index");
  }
}

void handleTaskControl(AsyncWebServerRequest *request) {
  if (!request->hasParam("task", true) || !request->hasParam("action", true)) {
    request->send(400, "text/plain", "Missing task or action parameter");
    return;
  }
  
  String taskName = request->getParam("task", true)->value();
  String action = request->getParam("action", true)->value();
  
  Serial.printf("Task control request: %s %s\n", taskName.c_str(), action.c_str());
  
  bool success = false;
  String response = "";
  
  if (taskName == "ir-learning") {
    if (action == "start") {
      success = taskManager.startIRLearningTask();
      response = success ? "✅ IR Learning task started" : "❌ Failed to start IR Learning task";
    } else if (action == "stop") {
      success = taskManager.stopIRLearningTask();
      response = success ? "✅ IR Learning task stopped" : "❌ Failed to stop IR Learning task";
    }
  } else if (taskName == "calibration") {
    if (action == "start") {
      success = taskManager.startCalibrationTask();
      response = success ? "✅ Calibration task started" : "❌ Failed to start Calibration task";
    } else if (action == "stop") {
      success = taskManager.stopCalibrationTask();
      response = success ? "✅ Calibration task stopped" : "❌ Failed to stop Calibration task";
    }
  } else {
    request->send(400, "text/plain", "Unknown task: " + taskName);
    return;
  }
  
  request->send(success ? 200 : 500, "text/plain", response);
}

void handleTaskStatus(AsyncWebServerRequest *request) {
  String status = taskManager.getTaskStatus();
  request->send(200, "application/json", status);
}

void handleStopTask(AsyncWebServerRequest *request) {
  if (!request->hasParam("task", true)) {
    request->send(400, "text/plain", "Missing task parameter");
    return;
  }
  
  String taskName = request->getParam("task", true)->value();
  bool success = false;
  
  if (taskName == "ir-learning") {
    success = taskManager.stopIRLearningTask();
  } else if (taskName == "calibration") {
    success = taskManager.stopCalibrationTask();
  }
  
  request->send(success ? 200 : 500, "text/plain", 
               success ? "Task stopped successfully" : "Failed to stop task");
}

void handleSettingsUpdate(AsyncWebServerRequest *request) {
  bool updated = false;
  
  if (request->hasParam("dayTemp", true) && request->hasParam("nightTemp", true)) {
    float dayTemp = request->getParam("dayTemp", true)->value().toFloat();
    float nightTemp = request->getParam("nightTemp", true)->value().toFloat();
    if (dayTemp > 0 && dayTemp < 40 && nightTemp > 0 && nightTemp < 40) {
      daySetting.temp = dayTemp;
      nightSetting.temp = nightTemp;
      updated = true;
      Serial.printf("Temperature settings updated: Day=%.1f°C, Night=%.1f°C\n", dayTemp, nightTemp);
    }
  }
  
  if (request->hasParam("dayWind", true) && request->hasParam("nightWind", true)) {
    int dayWind = request->getParam("dayWind", true)->value().toInt();
    int nightWind = request->getParam("nightWind", true)->value().toInt();
    if (dayWind >= 1 && dayWind <= 5 && nightWind >= 1 && nightWind <= 5) {
      daySetting.wind = dayWind;
      nightSetting.wind = nightWind;
      updated = true;
      Serial.printf("Wind settings updated: Day=%d, Night=%d\n", dayWind, nightWind);
    }
  }
  
  if (updated) {
    request->send(200, "text/plain", "Settings updated successfully");
  } else {
    request->send(400, "text/plain", "Invalid settings provided");
  }
}
