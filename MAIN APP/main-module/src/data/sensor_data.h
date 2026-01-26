#pragma once

#include <Arduino.h>

// =============================================================================
// STRUKTURY DANYCH - Inkubator Zakwasu
// =============================================================================

/**
 * Dane z sensora (temperatura, wilgotność, odległość)
 * Struktura musi być identyczna w nadajniku i odbiorniku!
 */
struct SensorData {
  float temperature;      // Temperatura [°C]
  float humidity;         // Wilgotność [%]
  float distance;         // Odległość/poziom zakwasu [cm]
  unsigned long timestamp; // Znacznik czasu
};

/**
 * Stan zakwasu - do przyszłej implementacji algorytmu
 */
enum class SourdoughState {
  UNKNOWN,
  FEEDING_NEEDED,     // Wymaga karmienia
  STATE_RISING,       // Rośnie
  PEAK,               // Szczyt aktywności
  STATE_FALLING,      // Opada
  READY_TO_USE,       // Gotowy do użycia
  OVERFERMENTED       // Przekwaszony
};

/**
 * Pełny stan systemu
 */
struct SystemState {
  SensorData sensorData;
  SourdoughState sourdoughState;
  bool sensorConnected;
  unsigned long lastDataTime;
  String wifiSSID;
  bool wifiConfigured;
};
