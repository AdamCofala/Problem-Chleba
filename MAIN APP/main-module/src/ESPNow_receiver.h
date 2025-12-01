#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "sensor_data.h"

class ESPNowReceiver {
private:
  static SensorData* receivedData;
  static bool* newDataFlag;
 
public:
  ESPNowReceiver() {}
  
  bool begin(SensorData* dataPtr, bool* flagPtr) {
    receivedData = dataPtr;
    newDataFlag = flagPtr;
    
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