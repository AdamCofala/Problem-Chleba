#pragma once

// =============================================================================
// OLED DISPLAY - Sourdough jar animation (SSD1306 128x64)
// =============================================================================
//
// This module displays an animated sourdough jar with rising bubbles
// on the OLED screen. The water/sourdough level can be adjusted based
// on distance sensor readings.
// =============================================================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "../config/pins.h"
#include "../config/settings.h"
#include "../data/sensor_data.h"
#include "animation.h"

class OLEDDisplay {
private:
  Adafruit_SSD1306 display;
  bool initialized;
  BubbleSystem bubbles;      // Bubble animation system
  JarRenderer jar;           // Jar graphics renderer
  unsigned long lastAnimationUpdate;

public:
  OLEDDisplay() :
    display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET),
    initialized(false),
    lastAnimationUpdate(0) {}

  /**
   * Initialize OLED display and animation system
   * @return true if successful
   */
  bool begin() {
    // Initialize I2C on configured pins
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println("ERROR: OLED initialization failed");
      return false;
    }

    display.clearDisplay();
    display.display();

    // Initialize bubble system with jar boundaries
    bubbles.init(
        jar.x + 6,           // Left boundary
        jar.x + jar.w - 6,   // Right boundary
        jar.y + 4,           // Top boundary
        jar.y + jar.h - 6    // Bottom boundary
    );

    initialized = true;
    Serial.printf("OLED initialized (SDA=%d, SCL=%d)\n", OLED_SDA, OLED_SCL);
    Serial.println("Sourdough animation active!");
    return true;
  }

  /**
   * Update animation - call this in loop()
   * Uses frame limiting based on ANIMATION_FPS setting
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
   * Set the fill level of the jar
   * @param level Fill percentage 0-100 (0=empty, 100=full)
   */
  void setWaterLevel(int level) {
    // Convert percentage to pixel position
    // level=100 -> waterLevel=10 (full jar)
    // level=0 -> waterLevel=55 (empty jar)
    jar.waterLevel = map(constrain(level, 0, 100), 0, 100, 55, 10);
  }

private:
  /**
   * Render one animation frame
   */
  void updateAnimation() {
    if (!initialized) return;

    display.clearDisplay();

    // Update bubble physics
    bubbles.update();

    // Draw jar outline and fill
    jar.drawJar(display);

    // Draw all bubbles
    for (int i = 0; i < BubbleSystem::COUNT; i++) {
      auto &b = bubbles.bubbles[i];
      display.drawCircle(b.x, b.y, b.radius(), SSD1306_WHITE);
    }

    display.display();
  }
};
