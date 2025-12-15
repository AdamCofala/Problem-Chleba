#include "OLED_display.h"
#include "ESPNow_receiver.h"
#include "sensor_data.h"
#include "WiFiConfigurator.h"


// Inicjalizacja statycznych zmiennych
SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;

// Główna klasa aplikacji
class MainModule {
private:
  OLEDDisplay display;
  ESPNowReceiver espNow;
  SensorData currentData;
  WiFiConfigurator wifiCfg;
  bool newDataAvailable;
  unsigned long lastDataTime;
  const unsigned long dataTimeout = 10000; // 10 sekund timeout
  
public:
  MainModule() : newDataAvailable(false), lastDataTime(0) {}
  
  void begin() {

    wifiCfg.begin();  // Uruchom konfigurator Wi-Fi

    // Sprawdź, czy po pominięciu konfiguracji lub błędzie mamy zapisane dane
    if (wifiCfg.hasCredentials() && wifiCfg.getSSID() != "") {
        Serial.println("Łączenie z Wi-Fi...");
        WiFi.begin(
            wifiCfg.getSSID().c_str(),
            wifiCfg.getPassword().c_str()
        );

        unsigned long startTime = millis();
        const unsigned long timeout = 10000; // 10 sekund na połączenie

        while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
            delay(300);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            Serial.println(WiFi.localIP());

            Serial.println("Email do SMTP: " + wifiCfg.getEmail());
            Serial.println("SSID: " + wifiCfg.getSSID());
        } else {
            Serial.println("\nNie udało się połączyć z Wi-Fi. SMTP nie będzie działać.");
        }
    } else {
        Serial.println("Brak konfiguracji Wi-Fi. SMTP nie będzie działać.");
    }

    Serial.println("Inicjalizacja modułu głównego (OLED)...");

    if (!display.begin()) {
        Serial.println("Nie udało się zainicjalizować OLED!");
        while (1) delay(1000);
    }

    if (!espNow.begin(&currentData, &newDataAvailable)) {
        Serial.println("Nie udało się zainicjalizować ESP-NOW!");
        while (1) delay(1000);
    }

    Serial.println("Moduł główny gotowy!");
}

  
  void loop() {
    if (newDataAvailable) {
      newDataAvailable = false;
      lastDataTime = millis();
      display.showSensorData(currentData);
    }

    display.draw3DAnimation();
    
    // Sprawdź timeout
    if (millis() - lastDataTime > dataTimeout && lastDataTime > 0) {
      display.showNoData();
    }
    
    delay(100);
  }
  
private:
  void printMacAddress() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("Adres MAC tego ESP32 (UŻYJ GO W NADAJNIKU): ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
    Serial.println("Skopiuj ten adres do zmiennej receiverAddress w kodzie nadajnika!");
  }
};

// Instancja głównej klasy
MainModule mainModule;

void setup() {
  Serial.begin(115200);
  mainModule.begin();
}

void loop() {
  mainModule.loop();
}