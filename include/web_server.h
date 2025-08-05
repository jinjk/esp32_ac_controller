#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"

// Web server functions
void initWiFi();
void setupWebServer();
String getWebContent();
void handleSettingsUpdate(AsyncWebServerRequest *request);
void handleIRLearn(AsyncWebServerRequest *request);
void handleIRSend(AsyncWebServerRequest *request);
void handleTaskControl(AsyncWebServerRequest *request);
void handleTaskStatus(AsyncWebServerRequest *request);
void handleStopTask(AsyncWebServerRequest *request);

// Global web server object
extern AsyncWebServer server;

#endif
