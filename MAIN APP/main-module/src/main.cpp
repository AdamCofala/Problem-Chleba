/**
 * =============================================================================
 * SOURDOUGH INCUBATOR - Main Module
 * =============================================================================
 *
 * Main features:
 * - Receives sensor data via ESP-NOW from remote sensor module
 * - Displays readings on ST7789 LCD (temperature, humidity, distance)
 * - Shows sourdough animation on SSD1306 OLED
 * - Web panel for configuration and monitoring (http://zakwas.local)
 *
 * Architecture:
 * - WiFi runs in AP+STA mode (Access Point + Station simultaneously)
 * - This allows web server and ESP-NOW to work together
 * - User connects to AP "Zakwas-Chlebowy" and accesses 192.168.4.1 or zakwas.local
 *
 * Project structure:
 * src/
 *   ├── config/       - Configuration (pins, settings)
 *   ├── data/         - Data structures
 *   ├── display/      - Displays (LCD, OLED)
 *   ├── comm/         - Communication (ESP-NOW)
 *   ├── web/          - Web server and HTML pages
 *   └── main.cpp      - Main application logic
 *
 * =============================================================================
 */

// Configuration
#include "config/pins.h"
#include "config/settings.h"

// Data structures
#include "data/sensor_data.h"

// Displays
#include "display/lcd_display.h"
#include "display/oled_display.h"

// Communication
#include "comm/espnow_receiver.h"

// Web server
#include "web/web_server.h"

// =============================================================================
// MAIN APPLICATION CLASS
// =============================================================================

class SourdoughIncubator {
private:
  // Hardware components
  LCDDisplay lcd;
  OLEDDisplay oled;
  ESPNowReceiver espNow;
  WebServerManager webServer;

  // System state
  SystemState systemState;
  bool newDataAvailable;
  unsigned long lastDisplayUpdate;

public:
  SourdoughIncubator() :
    newDataAvailable(false),
    lastDisplayUpdate(0) {
    // Initialize system state with defaults
    systemState.sensorData = {0, 0, 0, 0};
    systemState.sourdoughState = SourdoughState::UNKNOWN;
    systemState.sensorConnected = false;
    systemState.lastDataTime = 0;
    systemState.wifiConfigured = false;
    systemState.wifiConnected = false;
    systemState.sendingEmail = false;
  }

  /**
   * Initialize all system components
   * Called once from setup()
   */
  void begin() {
    Serial.begin(115200);
    delay(1000);

    // Step 1: Initialize displays
    Serial.println("[1/4] Initializing displays...");

    if (!lcd.begin()) {
      Serial.println("ERROR: LCD initialization failed!");
      while(1) delay(1000);
    }
    lcd.showWelcome();

    if (!oled.begin()) {
      Serial.println("ERROR: OLED initialization failed!");
      while(1) delay(1000);
    }

    // Step 2: Initialize web server (also configures WiFi in AP+STA mode)
    Serial.println("[2/4] Initializing web server...");
    if (!webServer.begin(&systemState)) {
      Serial.println("ERROR: Web server initialization failed!");
      lcd.showError("Web Server Error");
      while(1) delay(1000);
    }
    webServer.setLCD(&lcd);  // Pass LCD pointer for email status display

    // Step 3: Initialize ESP-NOW (must be after WiFi configuration)
    Serial.println("[3/4] Initializing ESP-NOW...");
    if (!espNow.begin(&systemState.sensorData, &newDataAvailable, &systemState.lastDataTime)) {
      Serial.println("ERROR: ESP-NOW initialization failed!");
      lcd.showError("ESP-NOW Error");
      while(1) delay(1000);
    }

    // Step 4: Ready!
    Serial.println("[4/4] System ready!");
    printStatus();

    lcd.showWaitingForData();
  }

  /**
   * Main loop - call from Arduino loop()
   */
  void loop() {
    // Handle web server requests
    webServer.loop();

    // Update OLED animation (runs continuously)
    oled.loop();

    // If email is being sent, show status and skip sensor processing
    if (systemState.sendingEmail) {
      static bool emailScreenShown = false;
      if (!emailScreenShown) {
        lcd.showSendingEmail();
        emailScreenShown = true;
      }
      return;  // Don't process sensor data during email send
    } else {
      static bool emailScreenShown = false;
      if (emailScreenShown) {
        emailScreenShown = false;
        lcd.forceRedraw();
      }
    }

    // Process new sensor data when available
    if (newDataAvailable) {
      newDataAvailable = false;
      systemState.sensorConnected = true;

      // Update LCD with sensor readings
      lcd.showSensorData(systemState.sensorData);

      // Update OLED animation level based on distance
      // Assumes distance 5-20cm maps to 0-100% fill level
      int level = map(constrain(systemState.sensorData.distance, 5, 20), 20, 5, 0, 100);
      oled.setWaterLevel(level);
    }

    // Check for sensor data timeout
    checkDataTimeout();
  }

private:
  /**
   * Check if sensor data has timed out (no data received recently)
   */
  void checkDataTimeout() {
    if (systemState.lastDataTime > 0) {
      unsigned long elapsed = millis() - systemState.lastDataTime;

      if (elapsed > DATA_TIMEOUT_MS) {
        if (systemState.sensorConnected) {
          systemState.sensorConnected = false;
          lcd.showNoData();
          Serial.println("WARNING: Sensor data timeout!");
        }
      }
    }
  }

  /**
   * Print startup banner to Serial
   */
  void printBanner() {
    Serial.println("\n");
    Serial.println("============================================");
    Serial.println("       SOURDOUGH INCUBATOR v1.0.0");
    Serial.println("============================================");
    Serial.println();
  }

  /**
   * Print system status to Serial
   */
  void printStatus() {
    Serial.println("\n--- System Status ---");
    Serial.printf("WiFi AP: %s\n", AP_SSID);
    Serial.printf("WiFi Password: %s\n", AP_PASSWORD);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("Web panel: http://zakwas.local");
    Serial.println("---------------------\n");
  }
};

// =============================================================================
// ARDUINO ENTRY POINTS
// =============================================================================

SourdoughIncubator incubator;

void setup() {
  incubator.begin();
}

void loop() {
  incubator.loop();
}
