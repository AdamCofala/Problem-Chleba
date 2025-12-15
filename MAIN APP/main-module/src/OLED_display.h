#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "sensor_data.h"
#include "animation.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// I2C buses
TwoWire I2C_SENSORS = TwoWire(0);
TwoWire I2C_ANIM    = TwoWire(1);

// -------------------------------------------------------------
// OLEDDisplay – 2 OLEDy, sensordata + animacja 3D
// -------------------------------------------------------------
class OLEDDisplay {
private:
    Adafruit_SSD1306 dspSensors;
    Adafruit_SSD1306 dspAnim;

    JarRenderer jar;
    BubbleSystem bubbles;

    bool initialized = false;

public:
    OLEDDisplay()
        : dspSensors(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_SENSORS, OLED_RESET),
          dspAnim   (SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_ANIM, OLED_RESET)
    {}

    bool begin() {
        // Start I2C
        I2C_SENSORS.begin(21,22);
        I2C_ANIM.begin(5,18);

        if (!dspSensors.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
            return false;
        if (!dspAnim.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
            return false;

        // Ustawienia
        dspSensors.clearDisplay();
        dspSensors.display();

        dspAnim.clearDisplay();
        dspAnim.display();

        // Inicjalizacja systemu bąbelków
        bubbles.init(
            jar.x + 6,
            jar.x + jar.w - 6,
            jar.y + 4,
            jar.y + jar.h - 6
        );

        initialized = true;
        return true;
    }
    void showNoData() {
      if (!initialized) return;
      
      dspSensors.clearDisplay();
      dspSensors.setCursor(0, 20);
      dspSensors.setTextSize(1);
      dspSensors.println("Brak danych");
      dspSensors.println("od sensora...");
      dspSensors.display();
    }
    
    void draw3DAnimation() {
        if (!initialized) return;

        dspAnim.clearDisplay();

        // Fizyka bąbelków
        bubbles.update();

        // Słoik
        jar.drawJar(dspAnim);

        // Bąbelki
        for (int i = 0; i < BubbleSystem::COUNT; i++) {
            auto &b = bubbles.bubbles[i];
            dspAnim.drawCircle(b.x, b.y, b.radius(), SSD1306_WHITE);
        }

        dspAnim.display();
    }
  
    void showSensorData(const SensorData &data) {
        if (!initialized) return;

        dspSensors.clearDisplay();
        dspSensors.setTextColor(SSD1306_WHITE);

        dspSensors.setCursor(0,0);
        dspSensors.setTextSize(1);
        dspSensors.print("Temp: ");
        dspSensors.setTextSize(2);
        dspSensors.println(data.temperature);

        dspSensors.setTextSize(1);
        dspSensors.print("Hum:  ");
        dspSensors.setTextSize(2);
        dspSensors.println(data.humidity);

        dspSensors.setTextSize(1);
        dspSensors.print("Dist: ");
        dspSensors.setTextSize(2);
        dspSensors.println(data.distance);

        dspSensors.display();
    }
};
