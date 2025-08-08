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
void handleSystemInfo(AsyncWebServerRequest *request);

// Rule management functions
void handleGetRules(AsyncWebServerRequest *request);
void handleCreateRule(AsyncWebServerRequest *request);
void handleUpdateRule(AsyncWebServerRequest *request);
void handleDeleteRule(AsyncWebServerRequest *request);
void handleGetActiveRule(AsyncWebServerRequest *request);

// Rule persistence functions
void handleSaveRules(AsyncWebServerRequest *request);
void handleLoadRules(AsyncWebServerRequest *request);
void handleResetRules(AsyncWebServerRequest *request);

// Global web server object
extern AsyncWebServer server;

#endif
