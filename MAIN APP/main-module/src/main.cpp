#include "OLED_display.h"
#include "ESPNow_receiver.h"
#include "sensor_data.h"
#include "WiFiConfig.h"
#include "animation.h"

// Inicjalizacja statycznych zmiennych
SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;

// Tryby pracy
enum OperationMode {
  MODE_WIFI_CONFIG,
  MODE_ESPNOW_RECEIVER
};

// Główna klasa aplikacji
class MainModule {
private:
  OLEDDisplay display;
  ESPNowReceiver espNow;
  WiFiConfig wifiConfig;
  SensorData currentData;
  bool newDataAvailable;
  unsigned long lastDataTime;
  unsigned long configStartTime;
  unsigned long pausedTime; // Czas spędzony w pauzie
  const unsigned long dataTimeout = 10000; // 10 sekund timeout
  const unsigned long configTimeout = 90000; // 90 sekund (1.5 minuty) na konfigurację
  OperationMode currentMode;
  bool espNowInitialized;
  bool wasClientConnected;
  
public:
  MainModule() : 
    newDataAvailable(false), 
    lastDataTime(0),
    configStartTime(0),
    pausedTime(0),
    currentMode(MODE_WIFI_CONFIG),
    espNowInitialized(false),
    wasClientConnected(false) {}
  
  void begin() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=========================");
    Serial.println("Inicjalizacja modułu głównego (OLED)...");
    
    if (!display.begin()) {
      Serial.println("Nie udało się zainicjalizować OLED!");
      while(1) delay(1000);
    }
    
    Serial.println("Moduł główny gotowy!");
    printMacAddress();
    Serial.println("=========================\n");
    
    // Start w trybie konfiguracji WiFi
    startWiFiConfigMode();
  }
  
  void loop() {
    if (currentMode == MODE_WIFI_CONFIG) {
      handleWiFiConfigMode();
    } else {
      handleESPNowMode();
    }


  }
  
private:
  void startWiFiConfigMode() {
    Serial.println("\n>>> Uruchamianie trybu konfiguracji WiFi");
    Serial.println(">>> Masz 90 sekund na konfigurację");
    Serial.println(">>> Timer zatrzyma się gdy ktoś się połączy");
    
    display.showConfigMode();
    
    if (!wifiConfig.begin()) {
      Serial.println("Błąd inicjalizacji WiFi Config!");
      display.showError("WiFi Error");
      delay(3000);
      switchToESPNowMode();
      return;
    }
    
    configStartTime = millis();
    pausedTime = 0;
    wasClientConnected = false;
    currentMode = MODE_WIFI_CONFIG;
  }
  
  void handleWiFiConfigMode() {
    wifiConfig.loop();
    
    // Sprawdź czy klient jest podłączony
    bool isClientConnected = (WiFi.softAPgetStationNum() > 0);
    
    // Obsługa stanu połączenia
    static unsigned long pauseStartTime = 0;
    
    if (isClientConnected && !wasClientConnected) {
      // Klient się właśnie podłączył - zatrzymaj timer
      pauseStartTime = millis();
      wasClientConnected = true;
      Serial.println(">>> Klient połączony - timer zatrzymany");
    } else if (!isClientConnected && wasClientConnected) {
      // Klient się rozłączył - wznów timer
      pausedTime += (millis() - pauseStartTime);
      wasClientConnected = false;
      Serial.println(">>> Klient rozłączony - timer wznowiony");
    }
    
    // Oblicz rzeczywisty upłynięty czas (z uwzględnieniem pauzy)
    unsigned long actualElapsed;
    if (isClientConnected) {
      // Timer jest zatrzymany - użyj czasu z momentu zatrzymania
      actualElapsed = pauseStartTime - configStartTime - pausedTime;
    } else {
      // Timer jest aktywny - odejmij łączny czas pauzy
      actualElapsed = millis() - configStartTime - pausedTime;
    }
    
    // Sprawdź czy użytkownik zakończył konfigurację lub timeout
    if (wifiConfig.isConfigComplete() || actualElapsed >= configTimeout) {
      if (wifiConfig.isConfigComplete()) {
        Serial.println("Konfiguracja zakończona przez użytkownika");
      } else {
        Serial.println("Timeout konfiguracji (90s)");
      }
      switchToESPNowMode();
      return;
    }
    
    // Aktualizuj wyświetlacz co sekundę
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
      lastDisplayUpdate = millis();
      
      int remainingSeconds = (configTimeout - actualElapsed) / 1000;
      
      if (isClientConnected) {
        display.showConfigPaused(remainingSeconds);
      } else {
        display.showConfigCountdown(remainingSeconds);
      }
    }
  }
  
  void switchToESPNowMode() {
    Serial.println("\n>>> Przełączanie na tryb odbiornika ESP-NOW");
    
    // Zatrzymaj WiFi Config
    wifiConfig.stop();
    
    // Wyłącz WiFi całkowicie
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    
    // Inicjalizuj ESP-NOW
    if (!espNow.begin(&currentData, &newDataAvailable)) {
      Serial.println("Nie udało się zainicjalizować ESP-NOW!");
      display.showError("Błąd ESP-NOW");
      while(1) delay(1000);
    }
    
    espNowInitialized = true;
    currentMode = MODE_ESPNOW_RECEIVER;
    lastDataTime = millis();
    
    display.showWaitingForData();
    Serial.println(">>> Tryb ESP-NOW aktywny\n");
  }
  
  void handleESPNowMode() {
    if (newDataAvailable) {
      newDataAvailable = false;
      lastDataTime = millis();
      display.showSensorData(currentData);
    }
    
    // Sprawdź timeout
    if (millis() - lastDataTime > dataTimeout && lastDataTime > 0) {
      display.showNoData();
    }
    
    delay(100);
  }
  
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
  mainModule.begin();
}

void loop() {
  mainModule.loop();
}