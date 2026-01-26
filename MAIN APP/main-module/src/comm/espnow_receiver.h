#pragma once

#include <esp_now.h>
#include <WiFi.h>

#include "../data/sensor_data.h"
#include <esp_wifi.h>

// =============================================================================
// ESP-NOW RECEIVER - Odbieranie danych z sensora
// =============================================================================

class ESPNowReceiver {
private:
  static SensorData* receivedData;
  static bool* newDataFlag;
  static unsigned long* lastDataTime;

public:
  ESPNowReceiver() {}

  /**
   * Inicjalizacja odbiornika ESP-NOW
   * UWAGA: WiFi musi być już w trybie WIFI_AP_STA przed wywołaniem!
   *
   * @param dataPtr Wskaźnik do struktury danych sensora
   * @param flagPtr Wskaźnik do flagi nowych danych
   * @param timePtr Wskaźnik do timestampa ostatnich danych
   * @return true jeśli sukces
   */
  bool begin(SensorData* dataPtr, bool* flagPtr, unsigned long* timePtr = nullptr) {
    receivedData = dataPtr;
    newDataFlag = flagPtr;
    lastDataTime = timePtr;

    // Sprawdź czy WiFi jest już w trybie AP+STA
    wifi_mode_t currentMode;
    esp_wifi_get_mode(&currentMode);

    if (currentMode != WIFI_MODE_APSTA) {
      Serial.println("UWAGA: WiFi nie jest w trybie AP+STA!");
      Serial.println("ESP-NOW może nie działać poprawnie.");
    }

    // Inicjalizuj ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Błąd inicjalizacji ESP-NOW");
      return false;
    }

    // Zarejestruj callback odbierania
    esp_now_register_recv_cb(onDataReceived);

    Serial.println("ESP-NOW odbiornik zainicjalizowany");
    printMacAddress();

    return true;
  }

  /**
   * Wyświetl adres MAC urządzenia
   */
  static void printMacAddress() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("Adres MAC (dla nadajnika): ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }

private:
  /**
   * Callback wywoływany przy odbiorze danych
   */
  static void onDataReceived(const uint8_t* mac, const uint8_t* incomingData, int len) {
    if (len == sizeof(SensorData) && receivedData != nullptr) {
      memcpy(receivedData, incomingData, sizeof(SensorData));

      if (newDataFlag != nullptr) {
        *newDataFlag = true;
      }

      if (lastDataTime != nullptr) {
        *lastDataTime = millis();
      }

      // Debug log
      Serial.println("\n=== ESP-NOW: Dane odebrane ===");
      Serial.print("Od: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
      Serial.printf("Temperatura: %.1f°C\n", receivedData->temperature);
      Serial.printf("Wilgotność: %.1f%%\n", receivedData->humidity);
      Serial.printf("Odległość: %.1f cm\n", receivedData->distance);
      Serial.println("==============================");
    } else {
      Serial.printf("ESP-NOW: Nieprawidłowy rozmiar pakietu (%d != %d)\n",
                    len, sizeof(SensorData));
    }
  }
};

// Definicja statycznych zmiennych
SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;
unsigned long* ESPNowReceiver::lastDataTime = nullptr;
