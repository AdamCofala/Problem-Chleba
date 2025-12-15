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
    void handleClient(); // nowa metoda do wywoływania w pętli
    void stop();
    
private:
    WebServer server;
    Preferences prefs;

    void setupAP();
    void setupServer();
    void handleRoot();
    void handleSave();
    void handleSkip();
};
