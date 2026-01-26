#pragma once

#include <Arduino.h>

// =============================================================================
// DATA STRUCTURES - Sourdough Incubator
// =============================================================================

/**
 * Sensor data structure for ESP-NOW transmission
 *
 * IMPORTANT: This structure must be identical on both sender and receiver!
 * Any change here requires updating the sensor module as well.
 */
struct SensorData {
  float temperature;       // Temperature in Celsius
  float humidity;          // Relative humidity in %
  float distance;          // Sourdough level/distance in cm
  unsigned long timestamp; // Timestamp from sender
};

/**
 * Sourdough state enumeration
 *
 * Used for future implementation of sourdough activity analysis algorithm.
 * The state is determined by analyzing temperature, humidity, and level
 * changes over time.
 */
enum class SourdoughState {
  UNKNOWN,          // Initial state, not enough data
  FEEDING_NEEDED,   // Sourdough needs feeding
  STATE_RISING,     // Sourdough is rising (fermentation active)
  PEAK,             // Peak activity reached
  STATE_FALLING,    // Sourdough is falling (past peak)
  READY_TO_USE,     // Optimal time to use
  OVERFERMENTED     // Over-fermented, needs refreshing
};

/**
 * Complete system state structure
 *
 * Contains all runtime state including sensor readings,
 * sourdough analysis state, WiFi status, and operational flags.
 */
struct SystemState {
  SensorData sensorData;          // Latest sensor readings
  SourdoughState sourdoughState;  // Current sourdough state
  bool sensorConnected;           // True if sensor is sending data
  unsigned long lastDataTime;     // Timestamp of last received data
  String wifiSSID;                // Configured home WiFi SSID
  bool wifiConfigured;            // True if home WiFi is configured
  bool wifiConnected;             // True if connected to home WiFi (STA mode)
  bool sendingEmail;              // True during email send operation
};
