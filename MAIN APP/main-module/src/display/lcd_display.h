#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "../config/pins.h"
#include "../data/sensor_data.h"

// =============================================================================
// LCD DISPLAY - Wyświetlanie danych (ST7789)
// =============================================================================

class LCDDisplay {
private:
  Adafruit_ST7789 tft;
  bool initialized;
  SensorData lastData;
  bool firstDraw;

public:
  LCDDisplay() : tft(TFT_CS, TFT_DC, TFT_RST), initialized(false), firstDraw(true) {
    lastData.temperature = -999;
    lastData.humidity = -999;
    lastData.distance = -999;
  }

  /**
   * Inicjalizacja wyświetlacza LCD
   */
  bool begin() {
    tft.init(LCD_WIDTH, LCD_HEIGHT, SPI_MODE0);
    tft.setRotation(2);
    tft.fillScreen(ST77XX_BLACK);
    tft.setSPISpeed(60000000); // 60 MHz

    initialized = true;
    Serial.println("LCD ST7789 zainicjalizowany (Hardware SPI)");
    return true;
  }

  /**
   * Wyświetl ekran powitalny
   */
  void showWelcome() {
    if (!initialized) return;

    tft.fillScreen(ST77XX_BLACK);

    // Tytuł
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(20, 30);
    tft.println("INKUBATOR");
    tft.setCursor(40, 55);
    tft.println("ZAKWASU");

    // Ikona chleba
    tft.setTextSize(4);
    tft.setCursor(95, 100);
    tft.print((char)0x03); // symbol

    // Info
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(30, 180);
    tft.println("Polacz sie z WiFi:");
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(30, 195);
    tft.printf("SSID: %s", AP_SSID);
    tft.setCursor(30, 210);
    tft.println("IP: 192.168.4.1");

    firstDraw = true;
  }

  /**
   * Wyświetl dane z sensora
   */
  void showSensorData(const SensorData& data) {
    if (!initialized) return;

    bool tempChanged = (abs(data.temperature - lastData.temperature) > 0.05);
    bool humChanged = (abs(data.humidity - lastData.humidity) > 0.05);
    bool distChanged = (abs(data.distance - lastData.distance) > 0.05);

    if (firstDraw) {
      tft.fillScreen(ST77XX_BLACK);
      firstDraw = false;
    }

    int y = 20;

    // Temperatura
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

    // Wilgotność
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

    // Odległość
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
   * Wyświetl oczekiwanie na dane
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
   * Wyświetl brak danych
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
   * Wyświetl błąd
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
   * Wymuś odświeżenie ekranu przy następnym wywołaniu
   */
  void forceRedraw() {
    firstDraw = true;
  }
};
