#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -----------------------------
// DHT SENSOR
// -----------------------------
#define DHTPIN 4
#define DHTTYPE DHT11  // or DHT22
DHT dht(DHTPIN, DHTTYPE);

// -----------------------------
// OLED
// -----------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -----------------------------
// ULTRASONIC SENSOR
// -----------------------------
const int trigPin = 5;
const int echoPin = 18;

#define SOUND_SPEED 0.034 // cm/us

// -----------------------------
// ULTRASONIC FUNCTION
// -----------------------------
float get_distance() {
  long duration;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  float distance = duration * SOUND_SPEED / 2;
  return distance;
}

// -----------------------------
// OLED PRINT FUNCTION
// -----------------------------
void show_on_oled(float temp, float hum, float dist) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.printf("%.1f%cC", temp, 247);  // Temperature

  display.setTextSize(1);
  display.setCursor(0, 25);
  display.printf("Humidity: %.1f%%", hum);

  display.setCursor(0, 45);
  display.printf("Distance: %.1f cm", dist);

  display.display();
}

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(115200);

  // DHT
  dht.begin();

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println("Starting...");
  display.display();
  delay(1500);
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float dist = get_distance();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT error!");
    display.clearDisplay();
    display.setCursor(10, 25);
    display.println("Sensor error!");
    display.display();
    return;
  }

  Serial.printf("Temp: %.1f°C | Hum: %.1f%% | Dist: %.1f cm\n", t, h, dist);

  show_on_oled(t, h, dist);

  delay(300);
}
