#pragma once

// =============================================================================
// USTAWIENIA APLIKACJI - Inkubator Zakwasu
// =============================================================================

// --- WiFi Access Point ---
#define AP_SSID "Zakwas-Chlebowy"
#define AP_PASSWORD "chlebek123"

// --- Timeouty ---
#define DATA_TIMEOUT_MS 10000       // Timeout na dane z sensora (10s)
#define CONFIG_TIMEOUT_MS 90000     // Timeout konfiguracji WiFi (90s)

// --- Animacja ---
#define ANIMATION_FPS 25
#define ANIMATION_INTERVAL_MS (1000 / ANIMATION_FPS)

// --- Web Server ---
#define WEB_SERVER_PORT 80
#define API_UPDATE_INTERVAL_MS 1000  // Interwał odświeżania danych na stronie

// --- Preferences keys ---
#define PREF_NAMESPACE "zakwas-config"
#define PREF_KEY_SSID "ssid"
#define PREF_KEY_PASSWORD "password"
