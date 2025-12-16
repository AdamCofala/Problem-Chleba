#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class WiFiConfig {
private:
  WebServer server;
  Preferences prefs;
  bool configComplete;
  String ssid;
  String password;
  
public:
  WiFiConfig() : server(80), configComplete(false) {}
  
  bool begin() {
    // Załaduj zapisane dane
    prefs.begin("wifi-config", false);
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("password", "");
    prefs.end();
    
    // Uruchom Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Zakwas-Chlebowy", "chlebek123");
    
    Serial.println("\n>>> WiFi AP uruchomiony");
    Serial.println(">>> SSID: ESP32-Config");
    Serial.println(">>> Hasło: 12345678");
    Serial.print(">>> IP Access Point: ");
    Serial.println(WiFi.softAPIP());
    
    // Konfiguracja endpointów
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/skip", HTTP_GET, [this]() { handleSkip(); });
    server.on("/clear", HTTP_GET, [this]() { handleClear(); });
    
    server.begin();
    Serial.println(">>> Serwer WWW uruchomiony\n");
    
    return true;
  }
  
  void loop() {
    server.handleClient();
  }
  
  void stop() {
    server.stop();
    WiFi.softAPdisconnect(true);
    Serial.println(">>> WiFi Config zatrzymany");
  }
  
  bool isConfigComplete() {
    return configComplete;
  }
  
private:
  void handleRoot() {
    String html = R"(
<!DOCTYPE html>
<html lang='pl'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Konfiguracja ESP32</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; }
    .container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; text-align: center; }
    .form-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 5px; color: #555; font-weight: bold; }
    input { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
    .btn { width: 100%; padding: 12px; margin: 5px 0; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; font-weight: bold; }
    .btn-primary { background: #4CAF50; color: white; }
    .btn-primary:hover { background: #45a049; }
    .btn-secondary { background: #2196F3; color: white; }
    .btn-secondary:hover { background: #0b7dda; }
    .btn-danger { background: #f44336; color: white; }
    .btn-danger:hover { background: #da190b; }
    .info { background: #e3f2fd; padding: 10px; border-radius: 4px; margin-bottom: 15px; }
    .saved { color: #4CAF50; font-weight: bold; }
  </style>
</head>
<body>
  <div class='container'>
    <h1>🔧 Konfiguracja WiFi</h1>
    <div class='info'>
      <p><strong>Aktualne dane:</strong></p>
      <p>SSID: <span class='saved'>)" + (ssid.length() > 0 ? ssid : "(brak)") + R"(</span></p>
    </div>
    <form action='/save' method='POST'>
      <div class='form-group'>
        <label for='ssid'>Nazwa sieci WiFi (SSID):</label>
        <input type='text' id='ssid' name='ssid' value=')" + ssid + R"(' required>
      </div>
      <div class='form-group'>
        <label for='password'>Hasło:</label>
        <input type='password' id='password' name='password' value=')" + password + R"('>
      </div>
      <button type='submit' class='btn btn-primary'>💾 Zapisz i kontynuuj</button>
    </form>
    <button onclick='location.href="/skip"' class='btn btn-secondary'>⏭️ Pomiń konfigurację</button>
    <button onclick='if(confirm("Czy na pewno chcesz wyczyścić zapisane dane?")) location.href="/clear"' class='btn btn-danger'>🗑️ Wyczyść dane</button>
  </div>
</body>
</html>
)";
    server.send(200, "text/html", html);
  }
  
  void handleSave() {
    if (server.hasArg("ssid")) {
      ssid = server.arg("ssid");
      password = server.hasArg("password") ? server.arg("password") : "";
      
      // Zapisz do pamięci nieulotnej
      prefs.begin("wifi-config", false);
      prefs.putString("ssid", ssid);
      prefs.putString("password", password);
      prefs.end();
      
      Serial.println("\n>>> Dane WiFi zapisane:");
      Serial.println("    SSID: " + ssid);
      Serial.println("    Hasło: " + password);
      
      String html = R"(
<!DOCTYPE html>
<html lang='pl'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Zapisano</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; display: flex; align-items: center; justify-content: center; height: 100vh; }
    .container { max-width: 400px; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); text-align: center; }
    .success { color: #4CAF50; font-size: 48px; margin-bottom: 20px; }
    h2 { color: #333; }
    p { color: #666; }
  </style>
</head>
<body>
  <div class='container'>
    <div class='success'>✅</div>
    <h2>Dane zapisane!</h2>
    <p>ESP32 przełączy się teraz na tryb odbiornika.</p>
  </div>
</body>
</html>
)";
      server.send(200, "text/html", html);
      delay(1000);
      configComplete = true;
    }
  }
  
  void handleSkip() {
    Serial.println("\n>>> Konfiguracja pominięta przez użytkownika");
    
    String html = R"(
<!DOCTYPE html>
<html lang='pl'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Pominięto</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; display: flex; align-items: center; justify-content: center; height: 100vh; }
    .container { max-width: 400px; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); text-align: center; }
    .info { color: #2196F3; font-size: 48px; margin-bottom: 20px; }
    h2 { color: #333; }
    p { color: #666; }
  </style>
</head>
<body>
  <div class='container'>
    <div class='info'>⏭️</div>
    <h2>Konfiguracja pominięta</h2>
    <p>ESP32 przełączy się na tryb odbiornika.</p>
  </div>
</body>
</html>
)";
    server.send(200, "text/html", html);
    delay(1000);
    configComplete = true;
  }
  
  void handleClear() {
    prefs.begin("wifi-config", false);
    prefs.clear();
    prefs.end();
    
    ssid = "";
    password = "";
    
    Serial.println("\n>>> Dane WiFi wyczyszczone");
    
    String html = R"(
<!DOCTYPE html>
<html lang='pl'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Wyczyszczono</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; display: flex; align-items: center; justify-content: center; height: 100vh; }
    .container { max-width: 400px; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); text-align: center; }
    .warning { color: #f44336; font-size: 48px; margin-bottom: 20px; }
    h2 { color: #333; }
    p { color: #666; }
    a { display: inline-block; margin-top: 20px; padding: 10px 20px; background: #2196F3; color: white; text-decoration: none; border-radius: 4px; }
    a:hover { background: #0b7dda; }
  </style>
</head>
<body>
  <div class='container'>
    <div class='warning'>🗑️</div>
    <h2>Dane wyczyszczone!</h2>
    <p>Wszystkie zapisane dane WiFi zostały usunięte.</p>
    <a href='/'>Powrót do konfiguracji</a>
  </div>
</body>
</html>
)";
    server.send(200, "text/html", html);
  }
};