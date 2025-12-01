#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sensor_data.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
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