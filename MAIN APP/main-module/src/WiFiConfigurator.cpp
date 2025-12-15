#include "WiFiConfigurator.h"

WiFiConfigurator::WiFiConfigurator() : server(80) {}

void WiFiConfigurator::begin() {
    prefs.begin("config", false);
    setupAP();
    setupServer();
}

void WiFiConfigurator::handleClient() {
    server.handleClient();
}

bool WiFiConfigurator::hasCredentials() {
    String ssid = getSSID();
    return ssid.length() > 0; // Sprawdzamy czy SSID nie jest pusty
}

String WiFiConfigurator::getSSID() { return prefs.getString("ssid", ""); }
String WiFiConfigurator::getPassword() { return prefs.getString("pass", ""); }
String WiFiConfigurator::getEmail() { return prefs.getString("email", ""); }

void WiFiConfigurator::setupAP() {
    WiFi.disconnect(); 
    delay(100);
    
    // ZAWSZE AP_STA - pozwala na jednoczesne bycie klientem routera i własnym punktem dostępu
    WiFi.mode(WIFI_AP_STA);
    
    WiFi.softAP("Zakwas-Setup");
    Serial.println("AP started: Zakwas-Setup");
    Serial.print("IP Strony Konfiguracyjnej: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiConfigurator::setupServer() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/skip", HTTP_GET, [this]() { handleSkip(); });
    server.begin();
}

void WiFiConfigurator::handleRoot() {
    // (Kod HTML bez zmian - taki jak wcześniej)
    const char* page = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Zakwas Setup</title>
<style>
body { font-family: Arial; background: #f4f4f9; margin: 0; padding: 0; }
.container { max-width: 400px; margin: 20px auto; padding: 20px; background: white; border-radius: 8px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); }
h2 { color: #4CAF50; text-align: center; }
input { width: 100%; padding: 10px; margin: 8px 0; border-radius: 5px; border:1px solid #ccc; box-sizing: border-box; }
button { width: 100%; padding: 10px; margin: 8px 0; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
button:hover { background-color: #45a049; }
a { display:block; text-align:center; margin-top:10px; color: #555; text-decoration: none; }
</style>
</head>
<body>
<div class="container">
<h2>Konfiguracja WiFi Zakwasu</h2>
<form method="POST" action="/save">
<input name="ssid" placeholder="SSID WiFi" required>
<input name="pass" type="password" placeholder="Hasło WiFi" required>
<input name="email" type="email" placeholder="Twój email" required>
<button type="submit">Zapisz</button>
</form>
<a href="/skip">Wyczyść / Pomiń</a>
</div>
</body>
</html>
)";
    server.send(200, "text/html", page);
}

void WiFiConfigurator::handleSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    String email = server.arg("email");

    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.putString("email", email);

    server.send(200, "text/html", "<h2>Zapisano! Zrestartuj urzadzenie przyciskiem RST.</h2>");
}

void WiFiConfigurator::handleSkip() {
    prefs.putString("ssid", "");
    prefs.putString("pass", "");
    prefs.putString("email", "");
    server.send(200, "text/html", "<h2>Konfiguracja wyczyszczona.</h2>");
}

void WiFiConfigurator::stop() {
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    Serial.println("Serwer konfiguracji zatrzymany. Radio wyłączone.");
}