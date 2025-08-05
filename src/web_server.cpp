#include "web_server.h"
#include "task_manager.h"
#include "ir_control.h"

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

String getWebContent() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  String mode = (timeinfo->tm_hour >= daySetting.startHour && timeinfo->tm_hour < daySetting.endHour) ? "Day" : "Night";
  
  return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32-S3 AC Controller</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body { font-family: Arial, sans-serif; margin: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }
            .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); }
            .status { background: linear-gradient(135deg, #e3ffe7 0%, #d9e7ff 100%); padding: 20px; border-radius: 10px; margin-bottom: 25px; border-left: 5px solid #007bff; }
            .task-controller { background: linear-gradient(135deg, #fff3e0 0%, #f3e5f5 100%); padding: 20px; border-radius: 10px; margin-bottom: 25px; border-left: 5px solid #ff9800; }
            .task-item { background: white; margin: 10px 0; padding: 15px; border-radius: 8px; border: 1px solid #ddd; display: flex; justify-content: space-between; align-items: center; }
            .task-running { border-left: 4px solid #4caf50; background: #f8fff8; }
            .task-stopped { border-left: 4px solid #9e9e9e; background: #f5f5f5; }
            .form-group { margin-bottom: 15px; }
            label { display: block; margin-bottom: 5px; font-weight: bold; color: #333; }
            input { width: 100%; padding: 12px; border: 2px solid #ddd; border-radius: 8px; box-sizing: border-box; transition: border-color 0.3s; }
            input:focus { border-color: #007bff; outline: none; }
            .btn { padding: 12px 24px; border: none; border-radius: 8px; cursor: pointer; font-size: 16px; font-weight: bold; transition: all 0.3s; margin: 5px; }
            .btn-primary { background: linear-gradient(135deg, #007bff, #0056b3); color: white; }
            .btn-success { background: linear-gradient(135deg, #28a745, #1e7e34); color: white; }
            .btn-warning { background: linear-gradient(135deg, #ffc107, #e0a800); color: #212529; }
            .btn-danger { background: linear-gradient(135deg, #dc3545, #bd2130); color: white; }
            .btn:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.15); }
            .btn:disabled { opacity: 0.6; cursor: not-allowed; transform: none; }
            h2 { color: #333; text-align: center; margin-bottom: 30px; }
            h3 { color: #666; border-bottom: 2px solid #007bff; padding-bottom: 10px; margin-bottom: 20px; }
            .status-indicator { display: inline-block; width: 12px; height: 12px; border-radius: 50%; margin-right: 8px; }
            .status-running { background-color: #4caf50; animation: pulse 2s infinite; }
            .status-stopped { background-color: #9e9e9e; }
            @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
            .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
            @media (max-width: 768px) { .grid { grid-template-columns: 1fr; } }
        </style>
        <script>
            function updateStatus() {
                fetch('/status')
                .then(response => response.json())
                .then(data => {
                    updateTaskStatus('ir-learning', data.irLearning);
                    updateTaskStatus('calibration', data.calibration);
                    document.getElementById('system-info').innerHTML = 
                        `Free Memory: ${data.system.freeHeap} bytes | Active Tasks: ${data.system.activeTasks} | Uptime: ${Math.floor(data.system.uptime/1000)}s`;
                })
                .catch(error => console.error('Error:', error));
            }
            
            function updateTaskStatus(taskId, taskData) {
                const element = document.getElementById(taskId);
                const indicator = element.querySelector('.status-indicator');
                const status = element.querySelector('.task-status');
                const button = element.querySelector('.task-btn');
                
                if (taskData.state === 'running') {
                    element.className = 'task-item task-running';
                    indicator.className = 'status-indicator status-running';
                    status.textContent = `Running (${Math.floor(taskData.runtime/1000)}s)`;
                    button.textContent = 'Stop';
                    button.className = 'btn btn-danger task-btn';
                    button.disabled = false;
                } else {
                    element.className = 'task-item task-stopped';
                    indicator.className = 'status-indicator status-stopped';
                    status.textContent = `Stopped (Last: ${Math.floor(taskData.lastDuration/1000)}s)`;
                    button.textContent = 'Start';
                    button.className = 'btn btn-success task-btn';
                    button.disabled = false;
                }
            }
            
            function controlTask(taskName, action) {
                const formData = new FormData();
                formData.append('task', taskName);
                formData.append('action', action);
                
                fetch('/task', {
                    method: 'POST',
                    body: formData
                })
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    updateStatus();
                })
                .catch(error => console.error('Error:', error));
            }
            
            setInterval(updateStatus, 2000); // Update every 2 seconds
            window.onload = updateStatus;
        </script>
    </head>
    <body>
    <div class="container">
        <h2>�️ ESP32-S3 AC Controller</h2>
        
        <div class="status">
            <p><strong>📊 System Status:</strong></p>
            <p>🌡️ Temperature: )rawliteral" + String(currentTemp, 1) + R"rawliteral(°C</p>
            <p>❄️ AC Status: )rawliteral" + (acOn ? "🟢 ON" : "🔴 OFF") + R"rawliteral(</p>
            <p>🕐 Mode: )rawliteral" + mode + R"rawliteral(</p>
            <p>📡 IR Buttons: )rawliteral" + String(learningState.learnedButtons) + "/" + String(learningState.totalButtons) + " learned" + R"rawliteral(</p>
            <p id="system-info" style="font-size: 0.9em; color: #666;">Loading system info...</p>
        </div>
        
        <div class="task-controller">
            <h3>🎮 Task Controller</h3>
            
            <div id="ir-learning" class="task-item task-stopped">
                <div>
                    <span class="status-indicator status-stopped"></span>
                    <strong>📡 IR Learning</strong><br>
                    <small class="task-status">)rawliteral" + (learningState.isLearning ? 
                        "Learning " + String(buttonNames[learningState.currentButton]) : 
                        String(learningState.learnedButtons) + "/" + String(learningState.totalButtons) + " buttons learned") + R"rawliteral(</small>
                </div>
                <button class="btn btn-success task-btn" onclick="controlTask('ir-learning', this.textContent.toLowerCase())">Start</button>
            </div>
            
            <div id="calibration" class="task-item task-stopped">
                <div>
                    <span class="status-indicator status-stopped"></span>
                    <strong>🔧 Sensor Calibration</strong><br>
                    <small class="task-status">Stopped</small>
                </div>
                <button class="btn btn-success task-btn" onclick="controlTask('calibration', this.textContent.toLowerCase())">Start</button>
            </div>
        </div>
        
        <div class="grid">
            <div>
                <h3>☀️ Daytime Settings (8AM - 7PM)</h3>
                <form action="/set" method="post">
                    <div class="form-group">
                        <label>🌡️ Temperature (°C):</label>
                        <input name="dayTemp" value=")rawliteral" + String(daySetting.temp, 1) + R"rawliteral(" min="18" max="35" step="0.5" type="number">
                    </div>
                    <div class="form-group">
                        <label>💨 Wind Speed (1-5):</label>
                        <input name="dayWind" value=")rawliteral" + String(daySetting.wind) + R"rawliteral(" min="1" max="5" type="number">
                    </div>
                    <button type="submit" class="btn btn-primary">💾 Save Day Settings</button>
                </form>
            </div>
            
            <div>
                <h3>🌙 Nighttime Settings (7PM - 8AM)</h3>
                <form action="/set" method="post">
                    <div class="form-group">
                        <label>🌡️ Temperature (°C):</label>
                        <input name="nightTemp" value=")rawliteral" + String(nightSetting.temp, 1) + R"rawliteral(" min="18" max="35" step="0.5" type="number">
                    </div>
                    <div class="form-group">
                        <label>💨 Wind Speed (1-5):</label>
                        <input name="nightWind" value=")rawliteral" + String(nightSetting.wind) + R"rawliteral(" min="1" max="5" type="number">
                    </div>
                    <button type="submit" class="btn btn-primary">💾 Save Night Settings</button>
                </form>
            </div>
        </div>
        
        <div style="margin-top: 20px;">
            <h3>🎯 IR Button Test</h3>
            <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px;">
                <button class="btn btn-secondary" onclick="sendIRButton(0)">⚡ Power ON</button>
                <button class="btn btn-secondary" onclick="sendIRButton(1)">⭕ Power OFF</button>
                <button class="btn btn-secondary" onclick="sendIRButton(2)">🔥 Temp UP</button>
                <button class="btn btn-secondary" onclick="sendIRButton(3)">❄️ Temp DOWN</button>
                <button class="btn btn-secondary" onclick="sendIRButton(4)">💨 Fan LOW</button>
                <button class="btn btn-secondary" onclick="sendIRButton(5)">🌪️ Fan MED</button>
                <button class="btn btn-secondary" onclick="sendIRButton(6)">🌊 Fan HIGH</button>
                <button class="btn btn-secondary" onclick="sendIRButton(7)">❄️ Cool Mode</button>
                <button class="btn btn-secondary" onclick="sendIRButton(8)">🔥 Heat Mode</button>
                <button class="btn btn-secondary" onclick="sendIRButton(9)">🤖 Auto Mode</button>
                <button class="btn btn-secondary" onclick="sendIRButton(10)">↔️ Swing ON</button>
                <button class="btn btn-secondary" onclick="sendIRButton(11)">⏸️ Swing OFF</button>
                <button class="btn btn-secondary" onclick="sendIRButton(12)">⏰ Timer ON</button>
                <button class="btn btn-secondary" onclick="sendIRButton(13)">⏱️ Timer OFF</button>
            </div>
            <div id="ir-status" style="margin-top: 10px; padding: 10px; background: #f0f0f0; border-radius: 5px; display: none;"></div>
            
            <script>
                function sendIRButton(buttonIndex) {
                    fetch('/send_ir', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                        body: 'button=' + buttonIndex
                    })
                    .then(response => response.text())
                    .then(data => {
                        const status = document.getElementById('ir-status');
                        status.textContent = data;
                        status.style.display = 'block';
                        status.style.backgroundColor = data.includes('✅') ? '#d4edda' : '#f8d7da';
                        setTimeout(() => status.style.display = 'none', 3000);
                    })
                    .catch(error => console.error('Error:', error));
                }
            </script>
        </div>
    </div>
    </body>
    </html>
  )rawliteral";
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
