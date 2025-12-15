#include "OLED_display.h"
#include "ESPNow_receiver.h"
#include "sensor_data.h"
#include "WiFiConfigurator.h"

SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;

class MainModule {
private:
    OLEDDisplay display;
    ESPNowReceiver espNow;
    SensorData currentData;
    WiFiConfigurator wifiCfg;
    
    bool newDataAvailable = false;
    unsigned long lastDataTime = 0;
    
    // Logika okna konfiguracyjnego
    unsigned long configStartTime;
    const unsigned long configTimeout = 60000; // 25 sekund
    bool configModeActive = true;
    bool userInteracted = false;

public:
    void begin() {
        Serial.begin(115200);
        display.begin();
        display.showNoData(); // Wyświetl cokolwiek na start

        Serial.println("--- TRYB KONFIGURACJI (60s) ---");
        wifiCfg.begin(); 
        configStartTime = millis();
    }

    void loop() {
        if (configModeActive) {
            handleConfigMode();
        } else {
            handleNormalMode();
        }
    }

private:
    void handleConfigMode() {
        wifiCfg.handleClient();
        
        unsigned long elapsed = millis() - configStartTime;
        int remaining = (configTimeout - elapsed) / 1000;

        // Wyświetlanie odliczania na OLED
        static int lastSec = -1;
        if (remaining != lastSec && remaining >= 0) {
            Serial.printf("Pozostało %d sekund na wejście w konfigurację...\n", remaining);
            // Tu możesz dodać display.showConfigTimer(remaining) jeśli masz taką metodę
            lastSec = remaining;
        }

        // Warunek wyjścia z trybu konfiguracji:
        // 1. Minął czas ORAZ nikt nie wszedł na stronę
        // 2. UWAGA: Jeśli chcesz, by po wejściu na stronę czas stał w miejscu, 
        //    musiałbyś w WiFiConfigurator ustawiać flagę 'userInteracted'.
        
        if (elapsed >= configTimeout) {
            exitConfigMode();
        }
    }

    void exitConfigMode() {
        Serial.println("Zamykanie trybu konfiguracji...");
        wifiCfg.stop();
        configModeActive = false;

        // DOPIERO TERAZ inicjalizujemy ESP-NOW
        if (espNow.begin(&currentData, &newDataAvailable)) {
            Serial.println("ESP-NOW gotowe do pracy.");
        }

        // Jeśli są dane WiFi, łączymy się w tle
        if (wifiCfg.hasCredentials()) {
            WiFi.mode(WIFI_AP_STA); // Przywracamy tryb mieszany dla ESP-NOW + STA
            WiFi.begin(wifiCfg.getSSID().c_str(), wifiCfg.getPassword().c_str());
            Serial.println("Próba łączenia z WiFi w tle...");
        }
    }

    void handleNormalMode() {
        // Standardowa praca urządzenia
        if (newDataAvailable) {
            newDataAvailable = false;
            lastDataTime = millis();
            display.showSensorData(currentData);
        }

        display.draw3DAnimation();

        if (millis() - lastDataTime > 10000 && lastDataTime > 0) {
            display.showNoData();
        }
    }
};

MainModule mainModule;

void setup() {
    mainModule.begin();
}

void loop() {
    mainModule.loop();
}