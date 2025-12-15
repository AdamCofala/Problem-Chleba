#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class WiFiConfigurator {
public:
    WiFiConfigurator();
    void begin();
    bool hasCredentials();

    String getSSID();
    String getPassword();
    String getEmail();

private:
    WebServer server;
    Preferences prefs;

    void setupAP();
    void setupServer();

    void handleRoot();
    void handleSave();
    void handleSkip();
};
