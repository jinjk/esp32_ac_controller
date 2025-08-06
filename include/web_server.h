#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

// Forward declarations
class AsyncWebServerRequest;
class AsyncWebServer;

// Web server functions
void initWiFi();
void setupWebServer();
String getWebContent();
String readFile(String path);
void handleSettingsUpdate(AsyncWebServerRequest *request);
void handleIRLearn(AsyncWebServerRequest *request);
void handleIRSend(AsyncWebServerRequest *request);
void handleTaskControl(AsyncWebServerRequest *request);
void handleTaskStatus(AsyncWebServerRequest *request);
void handleStopTask(AsyncWebServerRequest *request);
void handleSystemInfo(AsyncWebServerRequest *request);
void handleSettings(AsyncWebServerRequest *request);

// Global web server object
extern AsyncWebServer server;

#endif
