#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 4
#define DHTTYPE DHT11  // lub DHT22

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 25);
  display.println("Starting...");
  display.display();
  delay(1500);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  display.clearDisplay();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    display.println("Sensor error!");
  } else {
    Serial.printf("Temp: %.1f°C  Hum: %.1f%%\n", t, h);

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(15, 10);
    display.printf("%.1f%cC", t, 247); // 247 = znak °

    display.setTextSize(1);
    display.setCursor(35, 45);
    display.printf("Humidity: %.1f%%", h);
  }

  display.display();
  delay(2500);  
}

