# ESP32 DHT Temperature & Humidity fetcher + OLED screen

## Opis

ESP32 odczytuje temperaturę i wilgotność z DHT11 i wyświetla w Serial Monitorze oraz na OLED'dzie. Działa samodzielnie po zasileniu.

Tutoriale z których korzystałem:
- https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
- https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
## Wymagania

* ESP32 Dev Module
* DHT11 lub DHT22
* Rezystor 10 kΩ (pull-up DATA → VCC)
* PlatformIO + VS Code

## Wiring

<img width="357" height="447" alt="image" src="https://github.com/user-attachments/assets/63e8e585-3669-46ec-aba6-02962dd0047d" />
<img width="362" height="283" alt="image" src="https://github.com/user-attachments/assets/bee6eff5-f0ef-4ad8-ab9a-82eecf0d29ae" />



## Popełnione błędy

* **NaN w odczytach** → złe połączenie DATA, brak rezystora 10 kΩ (musi być dokładnie tyle) , zbyt szybkie odczyty

* **Kszaczki w odczytach** → różne prędkości na IO (monitor <-> GPIO) ```monitor_speed = 115200 (.ini)```

* **CPU BOOT LOADER ERROR CZY COŚ*** → za słabo podpięte kabelki

## Uwagi!
* Sensor potrzebuje jakiejś chwili by się nastawić, gdyż samo dotykanie go zmienia parametry. Dodatkowo jakość podłączonych kabli wpływa znacząco na poprawność działania sensora. Sensor sam w sobie bardzo całkiem dokładny, porównując z moim innym termometerem, błąd pomiarowy to ok. 0.2°C


## PlatformIO lib_deps (Biblioteki zewnętrzne zainstalowane)
```
lib_deps = adafruit/DHT sensor library@^1.4.6
- cos do oleda
```


