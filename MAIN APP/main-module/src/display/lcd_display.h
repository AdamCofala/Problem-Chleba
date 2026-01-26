#pragma once

// =============================================================================
// LCD DISPLAY - Sensor data visualization (ST7789 240x240)
// =============================================================================
//
// This module handles the main LCD display showing temperature, humidity,
// and sourdough level readings. Uses Hardware SPI for fast updates.
// Only redraws sections that have changed to minimize flicker.
// =============================================================================

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "../config/pins.h"
#include "../config/settings.h"
#include "../data/sensor_data.h"

class LCDDisplay {
private:
  Adafruit_ST7789 tft;
  bool initialized;
  SensorData lastData;    // Cached data for change detection
  bool firstDraw;         // Force full redraw on first call

public:
  LCDDisplay() : tft(TFT_CS, TFT_DC, TFT_RST), initialized(false), firstDraw(true) {
    // Initialize with invalid values to force first draw
    lastData.temperature = -999;
    lastData.humidity = -999;
    lastData.distance = -999;
  }

  /**
   * Initialize the LCD display
   * @return true if successful
   */
  bool begin() {
    tft.init(LCD_WIDTH, LCD_HEIGHT, SPI_MODE0);
    tft.setRotation(2);
    tft.fillScreen(ST77XX_BLACK);
    tft.setSPISpeed(60000000);  // 60 MHz for fast updates

    initialized = true;
    Serial.println("LCD ST7789 initialized (Hardware SPI)");
    return true;
  }

  /**
   * Show welcome screen with WiFi connection info
   */
  void showWelcome() {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);

    // Title
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(20, 30);
    tft.println("INKUBATOR");
    tft.setCursor(40, 55);
    tft.println("ZAKWASU");

    // Bread icon
    tft.setTextSize(4);
    tft.setCursor(95, 100);
    tft.print((char)0x03);

    // Connection instructions
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(30, 180);
    tft.println("Connect to WiFi:");
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(30, 195);
    tft.printf("SSID: %s", AP_SSID);
    tft.setCursor(30, 210);
    tft.println("URL: zakwas.local");

    firstDraw = true;
  }

  /**
   * Display sensor readings
   * Only updates sections that have changed to reduce flicker
   *
   * @param data Current sensor data
   */
  void showSensorData(const SensorData& data) {
    if (!initialized) return;

    // Check what values have changed (threshold to avoid noise)
    bool tempChanged = (abs(data.temperature - lastData.temperature) > 0.05);
    bool humChanged = (abs(data.humidity - lastData.humidity) > 0.05);
    bool distChanged = (abs(data.distance - lastData.distance) > 0.05);

    // Full redraw on first call
    if (firstDraw) {
      tft.fillScreen(ST77XX_BLACK);
      firstDraw = false;
    }

    int y = 20;

    // Temperature section
    if (tempChanged || firstDraw) {
      tft.fillRect(0, y, LCD_WIDTH, 75, ST77XX_BLACK);

      tft.setTextColor(ST77XX_RED);
      tft.setTextSize(2);
      tft.setCursor(10, y);
      tft.print("Temperatura:");

      y += 25;
      if (data.temperature > -900) {
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(3);
        tft.setCursor(20, y);
        tft.print(data.temperature, 1);
        tft.print(" C  ");
      } else {
        tft.setTextColor(ST77XX_RED);
        tft.setTextSize(2);
        tft.setCursor(20, y);
        tft.print("Error  ");
      }
      lastData.temperature = data.temperature;
    }

    y = 95;

    // Humidity section
    if (humChanged || firstDraw) {
      tft.fillRect(0, y, LCD_WIDTH, 75, ST77XX_BLACK);

      tft.setTextColor(ST77XX_BLUE);
      tft.setTextSize(2);
      tft.setCursor(10, y);
      tft.print("Wilgotnosc:");

      y += 25;
      if (data.humidity > -900) {
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(3);
        tft.setCursor(20, y);
        tft.print(data.humidity, 1);
        tft.print(" %  ");
      } else {
        tft.setTextColor(ST77XX_RED);
        tft.setTextSize(2);
        tft.setCursor(20, y);
        tft.print("Error  ");
      }
      lastData.humidity = data.humidity;
    }

    y = 170;

    // Distance/level section
    if (distChanged || firstDraw) {
      tft.fillRect(0, y, LCD_WIDTH, 75, ST77XX_BLACK);

      tft.setTextColor(ST77XX_GREEN);
      tft.setTextSize(2);
      tft.setCursor(10, y);
      tft.print("Poziom:");

      y += 25;
      if (data.distance > 0) {
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(3);
        tft.setCursor(20, y);
        tft.print(data.distance, 1);
        tft.print(" cm  ");
      } else {
        tft.setTextColor(ST77XX_RED);
        tft.setTextSize(2);
        tft.setCursor(20, y);
        tft.print("Error  ");
      }
      lastData.distance = data.distance;
    }
  }

  /**
   * Show "waiting for sensor data" message
   */
  void showWaitingForData() {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.println("Oczekiwanie na");
    tft.setCursor(30, 125);
    tft.println("dane z sensora...");

    firstDraw = true;
  }

  /**
   * Show "no data from sensor" warning
   */
  void showNoData() {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.println("Brak danych");
    tft.setCursor(30, 125);
    tft.println("od sensora...");

    firstDraw = true;
  }

  /**
   * Show error message
   * @param message Error text to display
   */
  void showError(const char* message) {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.println(message);

    firstDraw = true;
  }

  /**
   * Show "sending email" status
   * Displayed during email transmission to inform user
   */
  void showSendingEmail() {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(20, 80);
    tft.println("Wysylanie");
    tft.setCursor(20, 105);
    tft.println("e-maila...");
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.setCursor(20, 150);
    tft.println("Laczenie z WiFi i SMTP");
    tft.setCursor(20, 165);
    tft.println("Prosze czekac...");

    firstDraw = true;
  }

  /**
   * Force full screen redraw on next showSensorData() call
   */
  void forceRedraw() {
    firstDraw = true;
  }
};
