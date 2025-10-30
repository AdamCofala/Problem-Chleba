# ESP32 with HC-SR04 Ultrasonic Sensor with OLED

## Opis

ESP32 odległość z sensora i wyświetla w Serial Monitorze oraz na OLED'dzie. Działa samodzielnie po zasileniu.

Tutoriale z których korzystałem:
- [https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/](https://randomnerdtutorials.com/esp32-hc-sr04-ultrasonic-arduino/)
- https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
## Wymagania

* Chęci do życia
* Sensor 
* OLED
* KABELKI
* PlatformIO + VS Code

## Wiring

<img width="362" height="283" alt="image" src="https://github.com/user-attachments/assets/ecc26f6e-0f9f-4ed8-bab7-df2249efeaa6" />

<img width="362" height="283" alt="image" src="https://github.com/user-attachments/assets/bee6eff5-f0ef-4ad8-ab9a-82eecf0d29ae" />

## Popełnione błędy

* **Kszaczki w odczytach** → różne prędkości na IO (monitor <-> GPIO) ```monitor_speed = 115200 (.ini)```

## Uwagi!
* Generalnie jak chce się robić by super dokładnie działał ten czujnik to należy uwzględnić temperaturę i wilgotność powietrza bo wtedy się zmienia prędkość dźwięku.

## PlatformIO lib_deps (Biblioteki zewnętrzne zainstalowane)
```
lib_deps = adafruit/Adafruit SSD1306@^2.5.15
```


