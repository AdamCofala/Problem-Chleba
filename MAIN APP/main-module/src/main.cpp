#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfiguracja OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Struktura danych - musi być identyczna jak w nadajniku
struct SensorData {
  float temperature;
  float humidity;
  float distance;
  unsigned long timestamp;
};

// Klasa do obsługi wyświetlacza OLED
class OLEDDisplay {
private:
  Adafruit_SSD1306 display;
  bool initialized;
  
public:
  OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), initialized(false) {}
  
  bool begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println("Błąd inicjalizacji OLED");
      return false;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Oczekiwanie na");
    display.println("dane...");
    display.display();
    
    initialized = true;
    Serial.println("OLED zainicjalizowany");
    return true;
  }
  
  void showSensorData(const SensorData& data) {
    if (!initialized) return;

    display.clearDisplay();

    int y = 0;

    // Temperatura
    display.setCursor(0, y);
    display.setTextSize(1);
    display.print("Temp: ");
    if (data.temperature > -900) {
        display.setTextSize(2);
        display.print(data.temperature, 1);
        display.println("C");
    } else {
        display.println("Error");
    }

    y += 22;  // <<< odstęp; 16 px wysokości tekstu size 2 + 6 px odstępu

    // Wilgotność
    display.setCursor(0, y);
    display.setTextSize(1);
    display.print("Hum: ");
    if (data.humidity > -900) {
        display.setTextSize(2);
        display.print(data.humidity, 1);
        display.println("%");
    } else {
        display.println("Error");
    }

    y += 22;

    // Odległość
    display.setCursor(0, y);
    display.setTextSize(1);
    display.print("Dist: ");
    if (data.distance > 0) {
        display.setTextSize(2);
        display.print(data.distance, 1);
        display.println("cm");
    } else {
        display.println("Error");
    }

    display.display();
  }

  void showError(const char* message) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println(message);
    display.display();
  }
  
  void showNoData() {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("Brak danych");
    display.println("od sensora...");
    display.display();
  }
};

// Klasa do obsługi ESP-NOW jako odbiornik
class ESPNowReceiver {
private:
  static SensorData* receivedData;
  static bool* newDataFlag;
  static OLEDDisplay* displayPtr;
  
public:
  ESPNowReceiver() {}
  
  bool begin(SensorData* dataPtr, bool* flagPtr, OLEDDisplay* oledPtr) {
    receivedData = dataPtr;
    newDataFlag = flagPtr;
    displayPtr = oledPtr;
    
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
      Serial.println("Błąd inicjalizacji ESP-NOW");
      return false;
    }
    
    esp_now_register_recv_cb(onDataReceived);
    Serial.println("ESP-NOW odbiornik zainicjalizowany");
    return true;
  }
  
  static void onDataReceived(const uint8_t* mac, const uint8_t* incomingData, int len) {
    if (len == sizeof(SensorData)) {
      memcpy(receivedData, incomingData, sizeof(SensorData));
      *newDataFlag = true;
      
      Serial.println("\n=== Otrzymano dane ===");
      Serial.print("Od: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
      Serial.print("Temperatura: ");
      Serial.println(receivedData->temperature);
      Serial.print("Wilgotność: ");
      Serial.println(receivedData->humidity);
      Serial.print("Odległość: ");
      Serial.println(receivedData->distance);
      Serial.println("===================");
    }
  }
};

// Inicjalizacja statycznych zmiennych
SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;
OLEDDisplay* ESPNowReceiver::displayPtr = nullptr;

// Główna klasa aplikacji
class MainModule {
private:
  OLEDDisplay display;
  ESPNowReceiver espNow;
  SensorData currentData;
  bool newDataAvailable;
  unsigned long lastDataTime;
  const unsigned long dataTimeout = 10000; // 10 sekund timeout
  
public:
  MainModule() : newDataAvailable(false), lastDataTime(0) {}
  
  void begin() {
    Serial.begin(115200);
    Serial.println("Inicjalizacja modułu głównego (OLED)...");
    
    if (!display.begin()) {
      Serial.println("Nie udało się zainicjalizować OLED!");
      while(1) delay(1000);
    }
    
    if (!espNow.begin(&currentData, &newDataAvailable, &display)) {
      Serial.println("Nie udało się zainicjalizować ESP-NOW!");
      display.showError("Błąd ESP-NOW");
      while(1) delay(1000);
    }
    
    Serial.println("Moduł główny gotowy!");
    printMacAddress();
  }
  
  void loop() {
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