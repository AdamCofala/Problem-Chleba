/**
 * =============================================================================
 * INKUBATOR ZAKWASU - Moduł Główny
 * =============================================================================
 *
 * Główne funkcje:
 * - Odbieranie danych z sensora przez ESP-NOW
 * - Wyświetlanie danych na LCD ST7789
 * - Animacja zakwasu na OLED SSD1306
 * - Panel WWW do konfiguracji i monitorowania
 *
 * Architektura:
 * - WiFi działa w trybie AP+STA (Access Point + Station)
 * - To pozwala na jednoczesne działanie serwera WWW i ESP-NOW
 * - Użytkownik łączy się z AP "Zakwas-Chlebowy" i wchodzi na 192.168.4.1
 *
 * Struktura projektu:
 * src/
 *   ├── config/       - Konfiguracja (piny, ustawienia)
 *   ├── data/         - Struktury danych
 *   ├── display/      - Wyświetlacze (LCD, OLED)
 *   ├── comm/         - Komunikacja (ESP-NOW)
 *   ├── web/          - Serwer WWW i strony HTML
 *   └── main.cpp      - Główna logika aplikacji
 *
 * =============================================================================
 */

// --- Konfiguracja ---
#include "config/pins.h"
#include "config/settings.h"

// --- Dane ---
#include "data/sensor_data.h"

// --- Wyświetlacze ---
#include "display/lcd_display.h"
#include "display/oled_display.h"

// --- Komunikacja ---
#include "comm/espnow_receiver.h"

// --- Serwer WWW ---
#include "web/web_server.h"

// =============================================================================
// GŁÓWNA KLASA APLIKACJI
// =============================================================================

class SourdoughIncubator {
private:
  // Komponenty
  LCDDisplay lcd;
  OLEDDisplay oled;
  ESPNowReceiver espNow;
  WebServerManager webServer;

  // Stan systemu
  SystemState systemState;
  bool newDataAvailable;
  unsigned long lastDisplayUpdate;

public:
  SourdoughIncubator() :
    newDataAvailable(false),
    lastDisplayUpdate(0) {
    // Inicjalizuj stan systemu
    systemState.sensorData = {0, 0, 0, 0};
    systemState.sourdoughState = SourdoughState::UNKNOWN;
    systemState.sensorConnected = false;
    systemState.lastDataTime = 0;
    systemState.wifiConfigured = false;
  }

  /**
   * Inicjalizacja wszystkich komponentów
   */
  void begin() {
    Serial.begin(115200);
    delay(1000);


    // 1. Inicjalizuj wyświetlacze
    Serial.println("[1/4] Inicjalizacja wyświetlaczy...");

    if (!lcd.begin()) {
      Serial.println("BŁĄD: Nie można zainicjalizować LCD!");
      while(1) delay(1000);
    }
    lcd.showWelcome();

    if (!oled.begin()) {
      Serial.println("BŁĄD: Nie można zainicjalizować OLED!");
      while(1) delay(1000);
    }

    // 2. Inicjalizuj serwer WWW (to też konfiguruje WiFi w trybie AP+STA)
    Serial.println("[2/4] Inicjalizacja serwera WWW...");
    if (!webServer.begin(&systemState)) {
      Serial.println("BŁĄD: Nie można uruchomić serwera WWW!");
      lcd.showError("Web Server Error");
      while(1) delay(1000);
    }

    // 3. Inicjalizuj ESP-NOW (po skonfigurowaniu WiFi)
    Serial.println("[3/4] Inicjalizacja ESP-NOW...");
    if (!espNow.begin(&systemState.sensorData, &newDataAvailable, &systemState.lastDataTime)) {
      Serial.println("BŁĄD: Nie można zainicjalizować ESP-NOW!");
      lcd.showError("ESP-NOW Error");
      while(1) delay(1000);
    }

    // 4. Gotowe!
    Serial.println("[4/4] System gotowy!");
    printStatus();

    lcd.showWaitingForData();
  }

  /**
   * Główna pętla - wywołuj w loop()
   */
  void loop() {
    // Obsługa serwera WWW
    webServer.loop();

    // Animacja OLED (działa zawsze)
    oled.loop();

    // Obsługa nowych danych z sensora
    if (newDataAvailable) {
      newDataAvailable = false;
      systemState.sensorConnected = true;

      // Aktualizuj LCD
      lcd.showSensorData(systemState.sensorData);

      // Aktualizuj poziom w animacji OLED (na podstawie odległości)
      // Zakładamy że odległość 5-20cm odpowiada poziomowi 0-100%
      int level = map(constrain(systemState.sensorData.distance, 5, 20), 20, 5, 0, 100);
      oled.setWaterLevel(level);
    }

    // Sprawdź timeout danych
    checkDataTimeout();
  }

private:
  /**
   * Sprawdź czy dane z sensora nie są zbyt stare
   */
  void checkDataTimeout() {
    if (systemState.lastDataTime > 0) {
      unsigned long elapsed = millis() - systemState.lastDataTime;

      if (elapsed > DATA_TIMEOUT_MS) {
        if (systemState.sensorConnected) {
          systemState.sensorConnected = false;
          lcd.showNoData();
          Serial.println("UWAGA: Timeout danych z sensora!");
        }
      }
    }
  }

  /**
   * Wyświetl banner startowy
   */
  void printBanner() {
    Serial.println("\n");
    Serial.println("╔═══════════════════════════════════════════╗");
    Serial.println("║       INKUBATOR ZAKWASU CHLEBOWEGO        ║");
    Serial.println("║              v1.0.0                       ║");
    Serial.println("╚═══════════════════════════════════════════╝");
    Serial.println();
  }

  /**
   * Wyświetl status systemu
   */
  void printStatus() {
    Serial.println("\n--- Status systemu ---");
    Serial.printf("WiFi AP: %s\n", AP_SSID);
    Serial.printf("WiFi Password: %s\n", AP_PASSWORD);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Panel WWW: http://192.168.4.1");
    Serial.println("----------------------\n");
  }
};

// =============================================================================
// INSTANCJA I FUNKCJE ARDUINO
// =============================================================================

SourdoughIncubator incubator;

void setup() {
  incubator.begin();
}

void loop() {
  incubator.loop();
}