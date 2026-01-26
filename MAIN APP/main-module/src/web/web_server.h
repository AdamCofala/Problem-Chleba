#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "../config/settings.h"
#include "../data/sensor_data.h"
#include "html_pages.h"

// =============================================================================
// WEB SERVER - Obsługa strony WWW i API
// =============================================================================

class WebServerManager {
private:
  WebServer server;
  Preferences prefs;
  SystemState* systemState;

  String savedSSID;
  String savedPassword;

public:
  WebServerManager() : server(WEB_SERVER_PORT), systemState(nullptr) {}

  /**
   * Inicjalizacja serwera WWW
   * @param state Wskaźnik do stanu systemu
   * @return true jeśli sukces
   */
  bool begin(SystemState* state) {
    systemState = state;

    // Załaduj zapisane dane WiFi
    loadWifiSettings();

    // Skonfiguruj Access Point w trybie AP+STA (pozwala na ESP-NOW)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.println("\n=== Web Server ===");
    Serial.printf("AP SSID: %s\n", AP_SSID);
    Serial.printf("AP Password: %s\n", AP_PASSWORD);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("==================\n");

    // Ustaw zapisane dane w stanie systemu
    if (systemState) {
      systemState->wifiSSID = savedSSID;
      systemState->wifiConfigured = savedSSID.length() > 0;
    }

    // Konfiguracja endpointów
    setupRoutes();

    server.begin();
    Serial.println("Serwer WWW uruchomiony na porcie 80");

    return true;
  }

  /**
   * Obsługa żądań - wywołuj w loop()
   */
  void loop() {
    server.handleClient();
  }

  /**
   * Pobierz zapisany SSID
   */
  String getSavedSSID() { return savedSSID; }

  /**
   * Pobierz zapisane hasło
   */
  String getSavedPassword() { return savedPassword; }

private:
  /**
   * Konfiguracja wszystkich endpointów
   */
  void setupRoutes() {
    // Strona główna
    server.on("/", HTTP_GET, [this]() {
      server.send_P(200, "text/html", INDEX_HTML);
    });

    // API - dane z sensora
    server.on("/api/data", HTTP_GET, [this]() {
      handleApiData();
    });

    // API - pobierz ustawienia WiFi
    server.on("/api/wifi", HTTP_GET, [this]() {
      handleGetWifi();
    });

    // API - zapisz ustawienia WiFi
    server.on("/api/wifi", HTTP_POST, [this]() {
      handlePostWifi();
    });

    // API - wyczyść ustawienia WiFi
    server.on("/api/wifi/clear", HTTP_POST, [this]() {
      handleClearWifi();
    });

    // Obsługa 404
    server.onNotFound([this]() {
      server.send(404, "text/plain", "Nie znaleziono");
    });
  }

  /**
   * Załaduj ustawienia WiFi z pamięci nieulotnej
   */
  void loadWifiSettings() {
    prefs.begin(PREF_NAMESPACE, true); // readonly
    savedSSID = prefs.getString(PREF_KEY_SSID, "");
    savedPassword = prefs.getString(PREF_KEY_PASSWORD, "");
    prefs.end();

    Serial.printf("Załadowano WiFi: SSID='%s'\n", savedSSID.c_str());
  }

  /**
   * Zapisz ustawienia WiFi do pamięci nieulotnej
   */
  void saveWifiSettings(const String& ssid, const String& password) {
    prefs.begin(PREF_NAMESPACE, false);
    prefs.putString(PREF_KEY_SSID, ssid);
    prefs.putString(PREF_KEY_PASSWORD, password);
    prefs.end();

    savedSSID = ssid;
    savedPassword = password;

    if (systemState) {
      systemState->wifiSSID = ssid;
      systemState->wifiConfigured = ssid.length() > 0;
    }

    Serial.printf("Zapisano WiFi: SSID='%s'\n", ssid.c_str());
  }

  /**
   * Wyczyść ustawienia WiFi
   */
  void clearWifiSettings() {
    prefs.begin(PREF_NAMESPACE, false);
    prefs.clear();
    prefs.end();

    savedSSID = "";
    savedPassword = "";

    if (systemState) {
      systemState->wifiSSID = "";
      systemState->wifiConfigured = false;
    }

    Serial.println("Wyczyszczono ustawienia WiFi");
  }

  // ==========================================================================
  // HANDLERY API
  // ==========================================================================

  /**
   * GET /api/data - Dane z sensora i stan systemu
   */
  void handleApiData() {
    JsonDocument doc;

    if (systemState) {
      doc["temperature"] = systemState->sensorData.temperature;
      doc["humidity"] = systemState->sensorData.humidity;
      doc["distance"] = systemState->sensorData.distance;
      doc["sensorConnected"] = systemState->sensorConnected;
      doc["sourdoughState"] = static_cast<int>(systemState->sourdoughState);
      doc["uptime"] = millis() / 1000;
    } else {
      doc["temperature"] = 0;
      doc["humidity"] = 0;
      doc["distance"] = 0;
      doc["sensorConnected"] = false;
      doc["uptime"] = millis() / 1000;
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  /**
   * GET /api/wifi - Pobierz zapisane ustawienia WiFi
   */
  void handleGetWifi() {
    JsonDocument doc;
    doc["ssid"] = savedSSID;
    // Hasło nie jest zwracane ze względów bezpieczeństwa
    doc["configured"] = savedSSID.length() > 0;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  /**
   * POST /api/wifi - Zapisz ustawienia WiFi
   */
  void handlePostWifi() {
    if (server.hasArg("plain")) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));

      if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      String ssid = doc["ssid"] | "";
      String password = doc["password"] | "";

      if (ssid.length() == 0) {
        server.send(400, "application/json", "{\"error\":\"SSID is required\"}");
        return;
      }

      saveWifiSettings(ssid, password);
      server.send(200, "application/json", "{\"message\":\"Ustawienia zapisane!\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"No data\"}");
    }
  }

  /**
   * POST /api/wifi/clear - Wyczyść ustawienia WiFi
   */
  void handleClearWifi() {
    clearWifiSettings();
    server.send(200, "application/json", "{\"message\":\"Ustawienia wyczyszczone!\"}");
  }
};
