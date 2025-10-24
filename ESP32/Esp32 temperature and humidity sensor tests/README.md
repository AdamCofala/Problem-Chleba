# ESP32 DHT Temperature & Humidity fetcher

## Opis

ESP32 odczytuje temperaturę i wilgotność z DHT11 i wyświetla w Serial Monitorze. Działa samodzielnie po zasileniu.

Tutorial: https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/


## Wymagania

* ESP32 Dev Module
* DHT11 lub DHT22
* Rezystor 10 kΩ (pull-up DATA → VCC)
* PlatformIO + VS Code

## Wiring

```
DHT VCC → 3.3V (lub 5V)
DHT GND → GND
DHT DATA → GPIO4
10 kΩ rezystor między DATA a VCC
```

## PlatformIO lib_deps
```
lib_deps =
  adafruit/DHT sensor library
```


## Popełnione błędy

* **NaN w odczytach** → złe połączenie DATA, brak rezystora 10 kΩ (musi być dokładnie tyle) , zbyt szybkie odczyty

* **Kszaczki w odczytach** -> różne prędkości na IO (monitor <-> GPIO)