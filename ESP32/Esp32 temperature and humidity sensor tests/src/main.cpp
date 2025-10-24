#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11  // lub DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temp: %.1f°C  Hum: %.1f%%\n", t, h);
  }

  delay(2500);  // minimum 2 sekundy dla DHT22
}
