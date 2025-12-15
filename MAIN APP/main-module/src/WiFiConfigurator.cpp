#include "WiFiConfigurator.h"

WiFiConfigurator::WiFiConfigurator()
: server(80) {}

void WiFiConfigurator::begin() {
    prefs.begin("config", false);

    // Jeśli brak danych lub użytkownik chce ponownie skonfigurować
    if (!hasCredentials()) {
        Serial.println("Brak danych WiFi - uruchamiam tryb konfiguracji");

        setupAP();      // Włącz tryb AP
        setupServer();  // Uruchom serwer www

        // Pętla konfiguracji – czekamy, aż użytkownik zapisze dane lub kliknie 'Pomiń'
        while (!hasCredentials()) {
            server.handleClient(); // obsługa żądań strony
            delay(10);
        }
    }

    // Jeśli mamy dane, spróbuj od razu połączyć się z Wi-Fi
    if (hasCredentials() && getSSID() != "") {
        Serial.println("Łączenie z Wi-Fi...");
        WiFi.begin(getSSID().c_str(), getPassword().c_str());

        unsigned long startTime = millis();
        const unsigned long timeout = 10000; // 10 sekund na połączenie

        while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
            delay(300);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            Serial.println(WiFi.localIP());
            Serial.println("Email do SMTP: " + getEmail());
        } else {
            Serial.println("\nNie udało się połączyć z Wi-Fi. Spróbuj ponownie przez stronę konfiguracji.");
            // Możemy wyczyścić dane, aby ponownie wymusić konfigurację
            prefs.remove("ssid");
            prefs.remove("pass");
            prefs.remove("email");
            ESP.restart();
        }
    } else {
        Serial.println("Brak konfiguracji Wi-Fi. SMTP nie będzie działać.");
    }
}

bool WiFiConfigurator::hasCredentials() {
    return prefs.isKey("ssid") && prefs.isKey("pass") && prefs.isKey("email");
}

String WiFiConfigurator::getSSID() {
    return prefs.getString("ssid", "");
}

String WiFiConfigurator::getPassword() {
    return prefs.getString("pass", "");
}

String WiFiConfigurator::getEmail() {
    return prefs.getString("email", "");
}

void WiFiConfigurator::setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Zakwas-Setup");

    Serial.println("AP started");
    Serial.println(WiFi.softAPIP());
}

void WiFiConfigurator::setupServer() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.on("/skip", HTTP_GET, [this]() { handleSkip(); });

    server.begin();
}

void WiFiConfigurator::handleRoot() {
    const char* page = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Zakwas Setup</title>
<style>
body { font-family: Arial, sans-serif; background: #f4f4f9; margin: 0; padding: 0; }
.container { max-width: 600px; margin: 20px auto; padding: 20px; background: white; border-radius: 8px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1); }
h2 { color: #4CAF50; font-size: 24px; text-align: center; }
input { width: 100%; padding: 10px; margin: 8px 0; border: 1px solid #ccc; border-radius: 5px; }
button { width: 100%; padding: 10px; margin: 8px 0; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
button:hover { background-color: #45a049; }
.error { color: red; text-align: center; font-size: 14px; margin-top: 10px; }
@media screen and (max-width: 600px) { .container { margin: 10px; } }
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
<p><a href="/skip">Pomiń konfigurację</a></p>
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

    if (WiFi.begin(ssid.c_str(), pass.c_str()) == WL_CONNECTED) {
        prefs.putString("ssid", ssid);
        prefs.putString("pass", pass);
        prefs.putString("email", email);
        server.send(200, "text/html", "<h2>Zapisano dane. Restart...</h2>");
        delay(1500);
        ESP.restart();
    } else {
        server.send(200, "text/html", "<div class='error'>Błąd połączenia WiFi. Spróbuj ponownie.</div>");
    }
}

void WiFiConfigurator::handleSkip() {
    prefs.putString("ssid", ""); // Pomijamy zapis danych Wi-Fi
    prefs.putString("pass", "");
    prefs.putString("email", "");
    server.send(200, "text/html", "<h2>Konfiguracja pominięta. Restart...</h2>");
    delay(1500);
    ESP.restart();
}
