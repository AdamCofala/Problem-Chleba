#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>

// Konfiguracja pinów
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define TRIG_PIN 5
#define ECHO_PIN 18

uint8_t receiverAddress[] = {0x58, 0xBF, 0x25, 0x18, 0x69, 0x1C};

// Struktura danych do wysyłania
struct SensorData {
  float temperature;
  float humidity;
  float distance;
  unsigned long timestamp;
};

// Klasa do obsługi DHT11
class TemperatureSensor {
private:
  DHT dht;
  
public:
  TemperatureSensor(uint8_t pin, uint8_t type) : dht(pin, type) {}
  
  void begin() {
    dht.begin();
  }
  
  float readTemperature() {
    float temp = dht.readTemperature();
    return isnan(temp) ? -999.0 : temp;
  }
  
  float readHumidity() {
    float hum = dht.readHumidity();
    return isnan(hum) ? -999.0 : hum;
  }
};

// Klasa do obsługi HC-SR04
class UltrasonicSensor {
private:
  uint8_t trigPin;
  uint8_t echoPin;
  
public:
  UltrasonicSensor(uint8_t trig, uint8_t echo) : trigPin(trig), echoPin(echo) {}
  
  void begin() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  }
  
  float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 30000);
    if (duration == 0) return -1.0;
    
    float distance = duration * 0.034 / 2;
    return distance;
  }
};

// Klasa do obsługi ESP-NOW
class ESPNowSender {
private:
  uint8_t* peerAddress;
  esp_now_peer_info_t peerInfo;
  
public:
  ESPNowSender(uint8_t* address) : peerAddress(address) {}
  
  bool begin() {
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
      Serial.println("Błąd inicjalizacji ESP-NOW");
      return false;
    }
    
    memcpy(peerInfo.peer_addr, peerAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Błąd dodawania peer");
      return false;
    }
    
    esp_now_register_send_cb(onDataSent);
    Serial.println("ESP-NOW zainicjalizowane");
    return true;
  }
  
  bool sendData(const SensorData& data) {
    esp_err_t result = esp_now_send(peerAddress, (uint8_t*)&data, sizeof(data));
    return result == ESP_OK;
  }
  
  static void onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
    Serial.print("Status wysyłki: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sukces" : "Błąd");
  }
};

// Główna klasa aplikacji
class SensorModule {
private:
  TemperatureSensor tempSensor;
  UltrasonicSensor distSensor;
  ESPNowSender espNow;
  SensorData data;
  unsigned long lastReadTime;
  const unsigned long readInterval = 2000; // 2 sekundy
  
public:
  SensorModule() 
    : tempSensor(DHT_PIN, DHT_TYPE),
      distSensor(TRIG_PIN, ECHO_PIN),
      espNow(receiverAddress),
      lastReadTime(0) {}
  
  void begin() {
    Serial.begin(115200);
    Serial.println("Inicjalizacja modułu sensorowego...");
    
    tempSensor.begin();
    distSensor.begin();
    
    if (!espNow.begin()) {
      Serial.println("Nie udało się zainicjalizować ESP-NOW!");
      while(1) delay(1000);
    }
    
    Serial.println("Moduł sensorowy gotowy!");
    printMacAddress();
  }
  
  void loop() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastReadTime >= readInterval) {
      lastReadTime = currentTime;
      
      readSensors();
      displayData();
      sendData();
    }
  }
  
private:
  void readSensors() {
    data.temperature = tempSensor.readTemperature();
    data.humidity = tempSensor.readHumidity();
    data.distance = distSensor.readDistance();
    data.timestamp = millis();
  }
  
  void displayData() {
    Serial.println("\n=== Odczyt czujników ===");
    Serial.print("Temperatura: ");
    Serial.print(data.temperature);
    Serial.println(" °C");
    
    Serial.print("Wilgotność: ");
    Serial.print(data.humidity);
    Serial.println(" %");
    
    Serial.print("Odległość: ");
    Serial.print(data.distance);
    Serial.println(" cm");
    Serial.println("=======================");
  }
  
  void sendData() {
    if (espNow.sendData(data)) {
      Serial.println("Dane wysłane");
    } else {
      Serial.println("Błąd wysyłania danych");
    }
  }
  
  void printMacAddress() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("Adres MAC tego ESP32: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
};

// Instancja głównej klasy
SensorModule sensorModule;

void setup() {
  sensorModule.begin();
}

void loop() {
  sensorModule.loop();
}