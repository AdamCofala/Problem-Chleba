#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "../config/pins.h"
#include "../config/settings.h"
#include "../data/sensor_data.h"
#include "animation.h"

// =============================================================================
// OLED DISPLAY - Animacja zakwasu (SSD1306)
// =============================================================================

class OLEDDisplay {
private:
  Adafruit_SSD1306 display;
  bool initialized;
  BubbleSystem bubbles;
  JarRenderer jar;
  unsigned long lastAnimationUpdate;

public:
  OLEDDisplay() :
    display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET),
    initialized(false),
    lastAnimationUpdate(0) {}

  /**
   * Inicjalizacja wyświetlacza OLED
   */
  bool begin() {
    // Inicjalizuj I2C na skonfigurowanych pinach
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println("Błąd inicjalizacji OLED");
      return false;
    }

    display.clearDisplay();
    display.display();

    // Inicjalizuj system bąbelków
    bubbles.init(
        jar.x + 6,
        jar.x + jar.w - 6,
        jar.y + 4,
        jar.y + jar.h - 6
    );

    initialized = true;
    Serial.printf("OLED zainicjalizowany (SDA=%d, SCL=%d)\n", OLED_SDA, OLED_SCL);
    Serial.println("Animacja zakwasu aktywna!");
    return true;
  }

  /**
   * Aktualizacja animacji - wywołuj w loop()
   */
  void loop() {
    if (!initialized) return;

    unsigned long currentTime = millis();
    if (currentTime - lastAnimationUpdate >= ANIMATION_INTERVAL_MS) {
      lastAnimationUpdate = currentTime;
      updateAnimation();
    }
  }

  /**
   * Ustaw poziom cieczy w słoiku
   * @param level Poziom 0-100 (procent wypełnienia)
   */
  void setWaterLevel(int level) {
    // Przekształć procent na pozycję piksela
    // level=100 → waterLevel=10 (pełny)
    // level=0 → waterLevel=55 (pusty)
    jar.waterLevel = map(constrain(level, 0, 100), 0, 100, 55, 10);
  }

private:
  void updateAnimation() {
    if (!initialized) return;

    display.clearDisplay();

    // Fizyka bąbelków
    bubbles.update();

    // Słoik
    jar.drawJar(display);

    // Bąbelki
    for (int i = 0; i < BubbleSystem::COUNT; i++) {
      auto &b = bubbles.bubbles[i];
      display.drawCircle(b.x, b.y, b.radius(), SSD1306_WHITE);
    }

    display.display();
  }
};
