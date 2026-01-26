#pragma once

// =============================================================================
// WEB SERVER - HTTP API and configuration panel for Sourdough Incubator
// =============================================================================
//
// This module provides:
// - WiFi Access Point configuration (AP+STA mode for ESP-NOW compatibility)
// - mDNS support (access via http://zakwas.local)
// - REST API for sensor data, WiFi settings, and email configuration
// - Test email sending via SMTP (Gmail)
//
// Important: ESP-NOW must be deinitialized before connecting to external WiFi
// for email sending, then restored afterwards. See sendTestEmail() for details.
// =============================================================================

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ESP_Mail_Client.h>
#include <esp_wifi.h>

#include "../config/settings.h"
#include "../data/sensor_data.h"
#include "../comm/espnow_receiver.h"
#include "../display/lcd_display.h"
#include "html_pages.h"

class WebServerManager {
private:
  WebServer server;
  Preferences prefs;
  SystemState* systemState;
  SMTPSession smtp;
  LCDDisplay* lcd;

  // Saved WiFi credentials for STA mode (connecting to home network)
  String savedSSID;
  String savedPassword;

  // Email settings (sender is hardcoded, only recipient is configurable)
  String smtpHost;
  int smtpPort;
  String emailUser;
  String emailRecipient;

public:
  WebServerManager() : server(WEB_SERVER_PORT), systemState(nullptr), lcd(nullptr) {}

  /**
   * Initialize the web server and WiFi Access Point
   *
   * @param state Pointer to system state structure
   * @return true if initialization successful
   */
  bool begin(SystemState* state) {
    systemState = state;

    // Load saved settings from non-volatile storage
    loadWifiSettings();
    loadEmailSettings();

    // Configure Access Point in AP+STA mode (required for ESP-NOW to work)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);

    // Start mDNS responder - allows access via http://zakwas.local
    if (MDNS.begin("zakwas")) {
      MDNS.addService("http", "tcp", 80);
      Serial.println("mDNS started: http://zakwas.local");
    }

    Serial.println("\n=== Web Server ===");
    Serial.printf("AP SSID: %s\n", AP_SSID);
    Serial.printf("AP Password: %s\n", AP_PASSWORD);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("URL: http://zakwas.local");
    Serial.println("==================\n");

    // Update system state with loaded WiFi config
    if (systemState) {
      systemState->wifiSSID = savedSSID;
      systemState->wifiConfigured = savedSSID.length() > 0;
    }

    // Configure HTTP routes
    setupRoutes();

    server.begin();
    Serial.println("Web server started on port 80");

    return true;
  }

  /**
   * Handle incoming HTTP requests - call this in loop()
   */
  void loop() {
    server.handleClient();
  }

  /**
   * Set LCD display pointer for showing status messages during email send
   */
  void setLCD(LCDDisplay* lcdPtr) {
    lcd = lcdPtr;
  }

  // Getters for saved settings
  String getSavedSSID() { return savedSSID; }
  String getSavedPassword() { return savedPassword; }
  String getEmailUser() { return emailUser; }
  String getEmailRecipient() { return emailRecipient; }
  String getSmtpHost() { return smtpHost; }
  int getSmtpPort() { return smtpPort; }

private:
  // ===========================================================================
  // HTTP Route Configuration
  // ===========================================================================

  void setupRoutes() {
    // Main page - serves the HTML UI
    server.on("/", HTTP_GET, [this]() {
      server.send_P(200, "text/html", INDEX_HTML);
    });

    // API: Get sensor data and system status
    server.on("/api/data", HTTP_GET, [this]() {
      handleApiData();
    });

    // API: Get saved WiFi settings
    server.on("/api/wifi", HTTP_GET, [this]() {
      handleGetWifi();
    });

    // API: Save WiFi settings
    server.on("/api/wifi", HTTP_POST, [this]() {
      handlePostWifi();
    });

    // API: Clear WiFi settings
    server.on("/api/wifi/clear", HTTP_POST, [this]() {
      handleClearWifi();
    });

    // API: Get email settings
    server.on("/api/email", HTTP_GET, [this]() {
      handleGetEmail();
    });

    // API: Save email settings and send test email
    server.on("/api/email", HTTP_POST, [this]() {
      handlePostEmail();
    });

    // 404 handler
    server.onNotFound([this]() {
      server.send(404, "text/plain", "Not found");
    });
  }

  // ===========================================================================
  // Settings Storage (Non-Volatile Memory)
  // ===========================================================================

  /**
   * Load WiFi credentials from NVS (Preferences)
   */
  void loadWifiSettings() {
    prefs.begin(PREF_NAMESPACE, true);  // read-only mode
    savedSSID = prefs.getString(PREF_KEY_SSID, "");
    savedPassword = prefs.getString(PREF_KEY_PASSWORD, "");
    prefs.end();

    Serial.printf("Loaded WiFi config: SSID='%s'\n", savedSSID.c_str());
  }

  /**
   * Load email settings from NVS
   * Note: Sender email and SMTP server are hardcoded constants
   */
  void loadEmailSettings() {
    prefs.begin(PREF_NAMESPACE, true);
    smtpHost = SMTP_HOST;
    smtpPort = SMTP_PORT;
    emailUser = AUTHOR_EMAIL;
    emailRecipient = prefs.getString(PREF_KEY_EMAIL_RECIPIENT, "");
    prefs.end();
  }

  /**
   * Save WiFi credentials to NVS
   */
  void saveWifiSettings(const String& ssid, const String& password) {
    prefs.begin(PREF_NAMESPACE, false);  // read-write mode
    prefs.putString(PREF_KEY_SSID, ssid);
    prefs.putString(PREF_KEY_PASSWORD, password);
    prefs.end();

    savedSSID = ssid;
    savedPassword = password;

    if (systemState) {
      systemState->wifiSSID = ssid;
      systemState->wifiConfigured = ssid.length() > 0;
    }

    Serial.printf("Saved WiFi config: SSID='%s'\n", ssid.c_str());
  }

  /**
   * Save email recipient to NVS
   */
  void saveEmailSettings(const String& recipient) {
    prefs.begin(PREF_NAMESPACE, false);
    prefs.putString(PREF_KEY_EMAIL_RECIPIENT, recipient);
    prefs.end();

    smtpHost = SMTP_HOST;
    smtpPort = SMTP_PORT;
    emailUser = AUTHOR_EMAIL;
    emailRecipient = recipient;

    Serial.println("Saved email settings (recipient only)");
  }

  /**
   * Clear all WiFi settings from NVS
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

    Serial.println("WiFi settings cleared");
  }

  /**
   * Clear email settings from NVS
   */
  void clearEmailSettings() {
    prefs.begin(PREF_NAMESPACE, false);
    prefs.remove(PREF_KEY_EMAIL_RECIPIENT);
    prefs.end();

    smtpHost = SMTP_HOST;
    smtpPort = SMTP_PORT;
    emailUser = AUTHOR_EMAIL;
    emailRecipient = "";
  }

  // ===========================================================================
  // API Handlers
  // ===========================================================================

  /**
   * GET /api/data - Returns sensor readings and system status as JSON
   */
  void handleApiData() {
    JsonDocument doc;
    bool wifi = (WiFi.status() == WL_CONNECTED);

    if (systemState) {
      systemState->wifiConnected = wifi;
      doc["temperature"] = systemState->sensorData.temperature;
      doc["humidity"] = systemState->sensorData.humidity;
      doc["distance"] = systemState->sensorData.distance;
      doc["sensorConnected"] = systemState->sensorConnected;
      doc["sourdoughState"] = static_cast<int>(systemState->sourdoughState);
      doc["uptime"] = millis() / 1000;
      doc["wifiConnected"] = wifi;
    } else {
      doc["temperature"] = 0;
      doc["humidity"] = 0;
      doc["distance"] = 0;
      doc["sensorConnected"] = false;
      doc["uptime"] = millis() / 1000;
      doc["wifiConnected"] = wifi;
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  /**
   * GET /api/wifi - Returns saved WiFi SSID (password not exposed for security)
   */
  void handleGetWifi() {
    JsonDocument doc;
    doc["ssid"] = savedSSID;
    doc["configured"] = savedSSID.length() > 0;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  /**
   * POST /api/wifi - Save WiFi credentials
   * Body: { "ssid": "...", "password": "..." }
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
   * POST /api/wifi/clear - Clear saved WiFi settings
   */
  void handleClearWifi() {
    clearWifiSettings();
    server.send(200, "application/json", "{\"message\":\"Ustawienia wyczyszczone!\"}");
  }

  /**
   * GET /api/email - Returns saved email recipient
   */
  void handleGetEmail() {
    JsonDocument doc;
    doc["recipient"] = emailRecipient;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  /**
   * POST /api/email - Save email settings and send test email
   * Body: { "recipient": "email@example.com" }
   */
  void handlePostEmail() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"No data\"}");
      return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    String recipient = doc["recipient"] | "";

    if (recipient.isEmpty()) {
      server.send(400, "application/json", "{\"error\":\"Recipient email required\"}");
      return;
    }

    saveEmailSettings(recipient);

    // Attempt to connect to saved WiFi and send test email
    String errorMsg;
    bool sent = sendTestEmail(errorMsg);

    JsonDocument resp;
    resp["sent"] = sent;
    resp["message"] = sent ? "Wyslano testowy e-mail" : errorMsg;
    resp["wifiConnected"] = systemState ? systemState->wifiConnected : (WiFi.status() == WL_CONNECTED);

    String response;
    serializeJson(resp, response);
    server.send(sent ? 200 : 500, "application/json", response);
  }

  // ===========================================================================
  // Email Sending
  // ===========================================================================

  /**
   * Connect to saved WiFi network with timeout
   *
   * @param timeoutMs Connection timeout in milliseconds
   * @param err Output string for error message
   * @return true if connected successfully
   */
  bool connectToSavedWiFi(uint32_t timeoutMs, String& err) {
    if (savedSSID.isEmpty()) {
      err = "Brak zapisanej sieci WiFi";
      return false;
    }

    if (WiFi.status() == WL_CONNECTED) {
      if (systemState) systemState->wifiConnected = true;
      return true;
    }

    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
      delay(200);
    }

    bool ok = WiFi.status() == WL_CONNECTED;
    if (systemState) systemState->wifiConnected = ok;
    if (!ok) {
      err = "Nie udalo sie polaczyc z WiFi";
    }
    return ok;
  }

  /**
   * Send a test email using saved settings
   *
   * IMPORTANT: This function temporarily disrupts ESP-NOW communication!
   * The sequence is:
   *   1. Deinitialize ESP-NOW
   *   2. Connect to external WiFi (STA mode)
   *   3. Send email via SMTP
   *   4. Disconnect from WiFi
   *   5. Restore AP mode and ESP-NOW
   *
   * @param err Output string for error message if sending fails
   * @return true if email sent successfully
   */
  bool sendTestEmail(String& err) {
    // Show status on LCD immediately
    if (lcd) {
      lcd->showSendingEmail();
    }

    // Set sending flag to pause sensor data processing
    if (systemState) {
      systemState->sendingEmail = true;
    }
    Serial.println("\n========== [EMAIL] STARTING ==========");

    // Validate settings
    if (emailRecipient.isEmpty()) {
      err = "Uzupelnij ustawienia e-mail";
      if (systemState) systemState->sendingEmail = false;
      Serial.println("[EMAIL] ERROR: No recipient");
      return false;
    }

    if (savedSSID.isEmpty()) {
      err = "Brak zapisanej sieci WiFi";
      if (systemState) systemState->sendingEmail = false;
      Serial.println("[EMAIL] ERROR: No WiFi SSID");
      return false;
    }

    // Step 1: Deinitialize ESP-NOW BEFORE connecting to WiFi
    // This is critical - ESP-NOW and STA WiFi connection can conflict
    Serial.println("[EMAIL] Step 1: Deinitializing ESP-NOW...");
    esp_now_deinit();
    delay(100);
    Serial.println("[EMAIL] ESP-NOW disabled");

    // Step 2: Connect to home WiFi network
    Serial.println("[EMAIL] Step 2: Connecting to WiFi...");
    Serial.printf("[EMAIL] SSID: %s\n", savedSSID.c_str());

    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
      err = "Nie udalo sie polaczyc z WiFi";
      Serial.println("[EMAIL] ERROR: WiFi timeout");
      restoreESPNow();
      return false;
    }
    Serial.printf("[EMAIL] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    if (systemState) systemState->wifiConnected = true;

    // Step 3: Configure and send email via SMTP
    Serial.println("[EMAIL] Step 3: Sending email...");

    String pass = AUTHOR_PASSWORD;
    MailClient.networkReconnect(true);
    smtp.debug(1);

    Session_Config config;
    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = pass.c_str();
    config.login.user_domain = "";
    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 1;
    config.time.day_light_offset = 0;

    SMTP_Message message;
    message.sender.name = F("Zakwas");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = F("Test - Inkubator Zakwasu");
    message.addRecipient(F("Opiekun"), emailRecipient.c_str());

    String htmlMsg = F("<div style='font-family:Arial;background:#1a1410;color:#f5e6d3;padding:16px;border-radius:8px;'>"
                       "<h2>🍞 Test e-mail z Inkubatora Zakwasu</h2>"
                       "<p>Polaczenie z WiFi oraz SMTP zakonczone sukcesem.</p>"
                       "</div>");
    message.html.content = htmlMsg.c_str();
    message.html.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

    bool success = false;

    if (!smtp.connect(&config)) {
      err = String("Blad polaczenia SMTP: ") + smtp.errorReason().c_str();
      Serial.printf("[EMAIL] SMTP connect error: %s\n", smtp.errorReason().c_str());
    } else if (!MailClient.sendMail(&smtp, &message)) {
      err = String("Blad wysylki: ") + smtp.errorReason().c_str();
      Serial.printf("[EMAIL] Send error: %s\n", smtp.errorReason().c_str());
    } else {
      success = true;
      Serial.println("[EMAIL] Email sent successfully!");
    }

    smtp.closeSession();

    // Step 4: Restore ESP-NOW and AP mode
    restoreESPNow();

    Serial.println("========== [EMAIL] DONE ==========\n");
    return success;
  }

  /**
   * Restore Access Point and ESP-NOW after email sending
   *
   * This must exactly replicate the initial WiFi/ESP-NOW setup sequence:
   *   1. Disconnect STA
   *   2. Turn off WiFi completely
   *   3. Set AP+STA mode
   *   4. Start SoftAP on correct channel
   *   5. Initialize ESP-NOW and register callback
   */
  void restoreESPNow() {
    Serial.println("\n[RESTORE] Step 4: Disconnecting WiFi STA...");
    WiFi.disconnect(true);
    delay(100);

    // Completely disable WiFi before reconfiguration
    Serial.println("[RESTORE] Step 5: Turning off WiFi...");
    WiFi.mode(WIFI_OFF);
    delay(500);  // Important: longer delay for stability

    Serial.println("[RESTORE] Step 6: Setting AP+STA mode...");
    WiFi.mode(WIFI_AP_STA);
    delay(100);

    Serial.printf("[RESTORE] Step 7: Starting SoftAP (channel %d)...\n", AP_CHANNEL);
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    delay(300);  // Wait for AP to stabilize
    Serial.printf("[RESTORE] AP IP: %s\n", WiFi.softAPIP().toString().c_str());

    // Verify WiFi channel
    uint8_t primaryChan = 0;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);
    Serial.printf("[RESTORE] Current WiFi channel: %d\n", primaryChan);

    // Restart mDNS
    MDNS.end();
    if (MDNS.begin("zakwas")) {
      MDNS.addService("http", "tcp", 80);
    }

    Serial.println("[RESTORE] Step 8: Initializing ESP-NOW...");
    esp_err_t result = esp_now_init();
    if (result != ESP_OK) {
      Serial.printf("[RESTORE] ERROR: esp_now_init() = %d\n", result);
    } else {
      esp_now_register_recv_cb(ESPNowReceiver::onDataReceivedWrapper);
      Serial.println("[RESTORE] ESP-NOW active!");
      delay(100);
      ESPNowReceiver::printMacAddress();
    }

    if (systemState) {
      systemState->wifiConnected = false;
      systemState->sendingEmail = false;
    }
    Serial.println("[RESTORE] Restoration complete\n");
  }
};
