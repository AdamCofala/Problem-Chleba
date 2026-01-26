#pragma once

// =============================================================================
// KONFIGURACJA PINÓW - Inkubator Zakwasu
// =============================================================================

// --- OLED Display (I2C) ---
#define OLED_SDA 21
#define OLED_SCL 19
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// --- LCD ST7789 (Hardware SPI) ---
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4

// --- Rozmiary ekranów ---
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
