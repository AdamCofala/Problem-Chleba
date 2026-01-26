#pragma once

// =============================================================================
// ESP-NOW RECEIVER - Wireless sensor data receiver
// =============================================================================
//
// This module handles receiving sensor data from the remote sensor module
// via ESP-NOW protocol. ESP-NOW provides low-latency, connectionless
// communication between ESP32 devices.
//
// Important: WiFi must be in AP+STA mode before initializing ESP-NOW.
// The sender module must be configured with this device's MAC address.
// =============================================================================

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "../data/sensor_data.h"

class ESPNowReceiver {
private:
  // Static pointers to shared data (required for callback)
  static SensorData* receivedData;
  static bool* newDataFlag;
  static unsigned long* lastDataTime;

public:
  ESPNowReceiver() {}

  /**
   * Initialize ESP-NOW receiver
   *
   * NOTE: WiFi must already be in WIFI_AP_STA mode before calling this!
   *
   * @param dataPtr   Pointer to SensorData structure to store received data
   * @param flagPtr   Pointer to flag that will be set when new data arrives
   * @param timePtr   Pointer to timestamp of last received data (optional)
   * @return true if initialization successful
   */
  bool begin(SensorData* dataPtr, bool* flagPtr, unsigned long* timePtr = nullptr) {
    receivedData = dataPtr;
    newDataFlag = flagPtr;
    lastDataTime = timePtr;

    // Verify WiFi is in AP+STA mode
    wifi_mode_t currentMode;
    esp_wifi_get_mode(&currentMode);

    if (currentMode != WIFI_MODE_APSTA) {
      Serial.println("WARNING: WiFi is not in AP+STA mode!");
      Serial.println("ESP-NOW may not work correctly.");
    }

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("ERROR: ESP-NOW initialization failed");
      return false;
    }

    // Register receive callback
    esp_now_register_recv_cb(onDataReceived);

    Serial.println("ESP-NOW receiver initialized");
    printMacAddress();

    return true;
  }

  /**
   * Print device MAC address (needed for sender configuration)
   */
  static void printMacAddress() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC Address (for sender): ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }

  /**
   * Public callback wrapper for external reinitialization
   * Used by web_server.h after email sending to restore ESP-NOW
   */
  static void onDataReceivedWrapper(const uint8_t* mac, const uint8_t* data, int len) {
    onDataReceived(mac, data, len);
  }

  /**
   * Reinitialize ESP-NOW (after WiFi mode change)
   *
   * @return true if reinitialization successful
   */
  static bool reinit() {
    Serial.println("\n[ESP-NOW] Deinitializing...");
    esp_now_deinit();
    delay(200);

    Serial.println("[ESP-NOW] Reinitializing...");
    if (esp_now_init() != ESP_OK) {
      Serial.println("[ESP-NOW] ERROR: Initialization failed");
      return false;
    }

    esp_now_register_recv_cb(onDataReceived);
    Serial.println("[ESP-NOW] Reinitialized successfully");
    delay(100);
    printMacAddress();
    return true;
  }

private:
  /**
   * Callback function called when data is received via ESP-NOW
   *
   * @param mac          MAC address of sender
   * @param incomingData Raw data bytes
   * @param len          Data length in bytes
   */
  static void onDataReceived(const uint8_t* mac, const uint8_t* incomingData, int len) {
    // Validate packet size matches SensorData structure
    if (len == sizeof(SensorData) && receivedData != nullptr) {
      // Copy received data to shared structure
      memcpy(receivedData, incomingData, sizeof(SensorData));

      // Set new data flag
      if (newDataFlag != nullptr) {
        *newDataFlag = true;
      }

      // Update timestamp
      if (lastDataTime != nullptr) {
        *lastDataTime = millis();
      }

      // Debug output
      Serial.println("\n=== ESP-NOW: Data Received ===");
      Serial.print("From: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
      Serial.printf("Temperature: %.1f C\n", receivedData->temperature);
      Serial.printf("Humidity: %.1f %%\n", receivedData->humidity);
      Serial.printf("Distance: %.1f cm\n", receivedData->distance);
      Serial.println("==============================");
    } else {
      Serial.printf("ESP-NOW: Invalid packet size (%d != %d)\n",
                    len, sizeof(SensorData));
    }
  }
};

// Static member definitions
SensorData* ESPNowReceiver::receivedData = nullptr;
bool* ESPNowReceiver::newDataFlag = nullptr;
unsigned long* ESPNowReceiver::lastDataTime = nullptr;
