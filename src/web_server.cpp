#include "web_server.h"
#include "task_manager.h"
#include "ir_control.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Forward declarations
void handleACControl(AsyncWebServerRequest *request);

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

  // REST API endpoints
  server.on("/api/ac/control", HTTP_POST, handleACControl);
  server.on("/api/settings", HTTP_POST, handleSettingsUpdate);
  server.on("/api/tasks", HTTP_POST, handleTaskControl);
  server.on("/api/tasks/status", HTTP_GET, handleTaskStatus);
  server.on("/api/tasks/stop", HTTP_POST, handleStopTask);
  
  // System and configuration APIs
  server.on("/api/system", HTTP_GET, handleSystemInfo);
  server.on("/api/settings", HTTP_GET, handleSettings);
  
  // Rule management APIs
  server.on("/api/rules", HTTP_GET, handleGetRules);
  server.on("/api/rules", HTTP_POST, handleCreateRule);
  server.on("/api/rules", HTTP_PUT, handleUpdateRule);
  server.on("/api/rules", HTTP_DELETE, handleDeleteRule);
  server.on("/api/rules/active", HTTP_GET, handleGetActiveRule);
  
  server.on("/api/health", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["status"] = "ok";
    doc["timestamp"] = millis();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

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
  
  return html;
}

void handleACControl(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (!request->hasParam("action", true)) {
    doc["success"] = false;
    doc["message"] = "Missing action parameter";
    doc["error"] = "MISSING_PARAMETER";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  String action = request->getParam("action", true)->value();
  bool success = false;
  
  if (action == "power_on") {
    greeAC.powerOn();
    success = true;
    doc["message"] = "AC powered ON";
  } else if (action == "power_off") {
    greeAC.powerOff();
    success = true;
    doc["message"] = "AC powered OFF";
  } else if (action == "temp_up") {
    uint8_t currentTemp = greeAC.getTemperature();
    if (currentTemp < 30) {
      greeAC.setTemperature(currentTemp + 1);
      greeAC.sendCommand();
      success = true;
      doc["message"] = "Temperature increased to " + String(currentTemp + 1) + "°C";
    } else {
      doc["message"] = "Temperature already at maximum (30°C)";
    }
  } else if (action == "temp_down") {
    uint8_t currentTemp = greeAC.getTemperature();
    if (currentTemp > 16) {
      greeAC.setTemperature(currentTemp - 1);
      greeAC.sendCommand();
      success = true;
      doc["message"] = "Temperature decreased to " + String(currentTemp - 1) + "°C";
    } else {
      doc["message"] = "Temperature already at minimum (16°C)";
    }
  } else if (action == "fan_cycle") {
    uint8_t currentFan = greeAC.getFanSpeed();
    uint8_t nextFan = (currentFan + 1) % 4; // 0=Auto, 1=Low, 2=Med, 3=High
    greeAC.setFanSpeed(nextFan);
    greeAC.sendCommand();
    success = true;
    String fanNames[] = {"Auto", "Low", "Medium", "High"};
    doc["message"] = "Fan speed set to " + String(fanNames[nextFan]);
  } else if (action == "swing_toggle") {
    bool currentSwing = greeAC.getSwingV();
    greeAC.setSwingV(!currentSwing);
    greeAC.sendCommand();
    success = true;
    doc["message"] = "Swing " + String(!currentSwing ? "ON" : "OFF");
  } else {
    doc["success"] = false;
    doc["message"] = "Unknown action: " + action;
    doc["error"] = "INVALID_ACTION";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  doc["success"] = success;
  doc["action"] = action;
  doc["acState"] = greeAC.getStateString();
  
  String response;
  serializeJson(doc, response);
  request->send(success ? 200 : 400, "application/json", response);
}

void handleTaskControl(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (!request->hasParam("task", true) || !request->hasParam("action", true)) {
    doc["success"] = false;
    doc["message"] = "Missing task or action parameter";
    doc["error"] = "MISSING_PARAMETERS";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  String taskName = request->getParam("task", true)->value();
  String action = request->getParam("action", true)->value();
  
  Serial.printf("Task control request: %s %s\n", taskName.c_str(), action.c_str());
  
  bool success = false;
  
  // Note: Calibration task removed. Only AC control task supported.
  if (taskName == "control") {
    if (action == "start") {
      success = taskManager.startControlTask();
      doc["message"] = success ? "AC Control task started" : "Failed to start AC Control task";
    } else if (action == "stop") {
      success = taskManager.stopControlTask();
      doc["message"] = success ? "AC Control task stopped" : "Failed to stop AC Control task";
    } else {
      doc["success"] = false;
      doc["message"] = "Invalid action for " + taskName + ". Use 'start' or 'stop'";
      doc["task"] = taskName;
      doc["action"] = action;
      doc["error"] = "INVALID_ACTION";
      String response;
      serializeJson(doc, response);
      request->send(400, "application/json", response);
      return;
    }
  } else {
    doc["success"] = false;
    doc["message"] = "Unknown task: " + taskName;
    doc["task"] = taskName;
    doc["error"] = "UNKNOWN_TASK";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  doc["success"] = success;
  doc["task"] = taskName;
  doc["action"] = action;
  
  String response;
  serializeJson(doc, response);
  request->send(success ? 200 : 500, "application/json", response);
}

void handleTaskStatus(AsyncWebServerRequest *request) {
  String status = taskManager.getTaskStatus();
  request->send(200, "application/json", status);
}

void handleStopTask(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (!request->hasParam("task", true)) {
    doc["success"] = false;
    doc["message"] = "Missing task parameter";
    doc["error"] = "MISSING_PARAMETER";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  String taskName = request->getParam("task", true)->value();
  bool success = false;
  
  // Note: Only control task stop is supported. IR learning and calibration removed.
  doc["success"] = false;
  doc["message"] = "Task stopping not supported. Only AC Control task runs continuously. (IR learning and calibration removed)";
  doc["task"] = taskName;
  doc["error"] = "TASK_STOPPING_NOT_SUPPORTED";
  String response;
  serializeJson(doc, response);
  request->send(400, "application/json", response);
}

void handleSettingsUpdate(AsyncWebServerRequest *request) {
  JsonDocument doc;
  bool updated = false;
  
  if (request->hasParam("dayTemp", true) && request->hasParam("nightTemp", true)) {
    float dayTemp = request->getParam("dayTemp", true)->value().toFloat();
    float nightTemp = request->getParam("nightTemp", true)->value().toFloat();
    if (dayTemp > 0 && dayTemp < 40 && nightTemp > 0 && nightTemp < 40) {
      daySetting.temp = dayTemp;
      nightSetting.temp = nightTemp;
      updated = true;
      doc["dayTemp"] = dayTemp;
      doc["nightTemp"] = nightTemp;
      Serial.printf("Temperature settings updated: Day=%.1f°C, Night=%.1f°C\n", dayTemp, nightTemp);
    } else {
      doc["success"] = false;
      doc["message"] = "Temperature values must be between 0-40°C";
      doc["error"] = "INVALID_TEMPERATURE_RANGE";
      String response;
      serializeJson(doc, response);
      request->send(400, "application/json", response);
      return;
    }
  }
  
  if (request->hasParam("dayWind", true) && request->hasParam("nightWind", true)) {
    int dayWind = request->getParam("dayWind", true)->value().toInt();
    int nightWind = request->getParam("nightWind", true)->value().toInt();
    if (dayWind >= 1 && dayWind <= 5 && nightWind >= 1 && nightWind <= 5) {
      daySetting.wind = dayWind;
      nightSetting.wind = nightWind;
      updated = true;
      doc["dayWind"] = dayWind;
      doc["nightWind"] = nightWind;
      Serial.printf("Wind settings updated: Day=%d, Night=%d\n", dayWind, nightWind);
    } else {
      doc["success"] = false;
      doc["message"] = "Wind speed must be between 1-5";
      doc["error"] = "INVALID_WIND_RANGE";
      String response;
      serializeJson(doc, response);
      request->send(400, "application/json", response);
      return;
    }
  }
  
  if (updated) {
    doc["success"] = true;
    doc["message"] = "Settings updated successfully";
  } else {
    doc["success"] = false;
    doc["message"] = "No valid parameters provided";
    doc["error"] = "NO_PARAMETERS";
  }
  
  String response;
  serializeJson(doc, response);
  request->send(doc["success"] ? 200 : 400, "application/json", response);
}

void handleSystemInfo(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Current system status
  doc["currentTemp"] = currentTemp;
  
  // Time and mode
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  String mode = (timeinfo->tm_hour >= daySetting.startHour && timeinfo->tm_hour < daySetting.endHour) ? "Day" : "Night";
  doc["mode"] = mode;
  doc["currentHour"] = timeinfo->tm_hour;
  
  // System info
  JsonObject system = doc["system"].to<JsonObject>();
  system["freeHeap"] = ESP.getFreeHeap();
  system["uptime"] = millis();
  system["activeTasks"] = uxTaskGetNumberOfTasks();
  system["chipCores"] = ESP.getChipCores();
  system["cpuFreq"] = ESP.getCpuFreqMHz();
  system["flashSize"] = ESP.getFlashChipSize();
  system["psramSize"] = ESP.getPsramSize();
  
  // IR status (Gree AC is always ready)
  JsonObject irStatus = doc["ir"].to<JsonObject>();
  irStatus["ready"] = true;  // Gree AC is always ready
  irStatus["type"] = "Gree AC Library";  // No IR learning - uses built-in library
  irStatus["receiver_required"] = false;  // No IR receiver needed
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleSettings(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Day settings
  JsonObject daySettings = doc["day"].to<JsonObject>();
  daySettings["temp"] = daySetting.temp;
  daySettings["wind"] = daySetting.wind;
  daySettings["startHour"] = daySetting.startHour;
  daySettings["endHour"] = daySetting.endHour;
  
  // Night settings
  JsonObject nightSettings = doc["night"].to<JsonObject>();
  nightSettings["temp"] = nightSetting.temp;
  nightSettings["wind"] = nightSetting.wind;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Rule management functions
void handleGetRules(AsyncWebServerRequest *request) {
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
  }
  
  doc["count"] = ruleCount;
  doc["activeRuleId"] = activeRuleId;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleCreateRule(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (ruleCount >= MAX_RULES) {
    doc["success"] = false;
    doc["message"] = "Maximum number of rules reached";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  // Get next available ID
  int newId = 1;
  for (int i = 0; i < ruleCount; i++) {
    if (rules[i].id >= newId) {
      newId = rules[i].id + 1;
    }
  }
  
  // Add new rule with default values
  rules[ruleCount] = {
    .id = newId,
    .name = "New Rule",
    .enabled = true,
    .startHour = -1,
    .endHour = -1,
    .minTemp = -999,
    .maxTemp = -999,
    .acOn = true,
    .setTemp = 25.0,
    .fanSpeed = 2,
    .mode = 0
  };
  
  ruleCount++;
  
  doc["success"] = true;
  doc["message"] = "Rule created successfully";
  doc["ruleId"] = newId;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleUpdateRule(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (!request->hasParam("id", true)) {
    doc["success"] = false;
    doc["message"] = "Rule ID required";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  int ruleId = request->getParam("id", true)->value().toInt();
  int ruleIndex = -1;
  
  // Find rule by ID
  for (int i = 0; i < ruleCount; i++) {
    if (rules[i].id == ruleId) {
      ruleIndex = i;
      break;
    }
  }
  
  if (ruleIndex == -1) {
    doc["success"] = false;
    doc["message"] = "Rule not found";
    String response;
    serializeJson(doc, response);
    request->send(404, "application/json", response);
    return;
  }
  
  // Update rule parameters
  if (request->hasParam("name", true)) {
    rules[ruleIndex].name = request->getParam("name", true)->value();
  }
  if (request->hasParam("enabled", true)) {
    rules[ruleIndex].enabled = request->getParam("enabled", true)->value() == "true";
  }
  if (request->hasParam("startHour", true)) {
    rules[ruleIndex].startHour = request->getParam("startHour", true)->value().toInt();
  }
  if (request->hasParam("endHour", true)) {
    rules[ruleIndex].endHour = request->getParam("endHour", true)->value().toInt();
  }
  if (request->hasParam("minTemp", true)) {
    rules[ruleIndex].minTemp = request->getParam("minTemp", true)->value().toFloat();
  }
  if (request->hasParam("maxTemp", true)) {
    rules[ruleIndex].maxTemp = request->getParam("maxTemp", true)->value().toFloat();
  }
  if (request->hasParam("acOn", true)) {
    rules[ruleIndex].acOn = request->getParam("acOn", true)->value() == "true";
  }
  if (request->hasParam("setTemp", true)) {
    rules[ruleIndex].setTemp = request->getParam("setTemp", true)->value().toFloat();
  }
  if (request->hasParam("fanSpeed", true)) {
    rules[ruleIndex].fanSpeed = request->getParam("fanSpeed", true)->value().toInt();
  }
  if (request->hasParam("mode", true)) {
    rules[ruleIndex].mode = request->getParam("mode", true)->value().toInt();
  }
  
  doc["success"] = true;
  doc["message"] = "Rule updated successfully";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleDeleteRule(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (!request->hasParam("id", true)) {
    doc["success"] = false;
    doc["message"] = "Rule ID required";
    String response;
    serializeJson(doc, response);
    request->send(400, "application/json", response);
    return;
  }
  
  int ruleId = request->getParam("id", true)->value().toInt();
  int ruleIndex = -1;
  
  // Find rule by ID
  for (int i = 0; i < ruleCount; i++) {
    if (rules[i].id == ruleId) {
      ruleIndex = i;
      break;
    }
  }
  
  if (ruleIndex == -1) {
    doc["success"] = false;
    doc["message"] = "Rule not found";
    String response;
    serializeJson(doc, response);
    request->send(404, "application/json", response);
    return;
  }
  
  // Shift remaining rules
  for (int i = ruleIndex; i < ruleCount - 1; i++) {
    rules[i] = rules[i + 1];
  }
  ruleCount--;
  
  doc["success"] = true;
  doc["message"] = "Rule deleted successfully";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleGetActiveRule(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  doc["activeRuleId"] = activeRuleId;
  doc["currentTemp"] = currentTemp;
  
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  doc["currentHour"] = timeinfo->tm_hour;
  
  if (activeRuleId != -1) {
    // Find and return active rule details
    for (int i = 0; i < ruleCount; i++) {
      if (rules[i].id == activeRuleId) {
        JsonObject activeRule = doc["activeRule"].to<JsonObject>();
        activeRule["id"] = rules[i].id;
        activeRule["name"] = rules[i].name;
        activeRule["acOn"] = rules[i].acOn;
        activeRule["setTemp"] = rules[i].setTemp;
        activeRule["fanSpeed"] = rules[i].fanSpeed;
        activeRule["mode"] = rules[i].mode;
        break;
      }
    }
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}
