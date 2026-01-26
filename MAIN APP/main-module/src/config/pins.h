#pragma once

// =============================================================================
// PIN CONFIGURATION - Sourdough Incubator
// =============================================================================

// -----------------------------------------------------------------------------
// OLED Display (SSD1306 via I2C)
// -----------------------------------------------------------------------------
#define OLED_SDA 21         // I2C Data pin
#define OLED_SCL 19         // I2C Clock pin
#define OLED_RESET -1       // Reset pin (-1 if not connected)
#define OLED_ADDRESS 0x3C   // I2C address (0x3C or 0x3D)

// -----------------------------------------------------------------------------
// LCD Display (ST7789 via Hardware SPI)
// -----------------------------------------------------------------------------
#define TFT_CS    15        // Chip Select pin
#define TFT_DC    2         // Data/Command pin
#define TFT_RST   4         // Reset pin

// Note: MOSI (23), SCLK (18), MISO (19) are default ESP32 SPI pins

// -----------------------------------------------------------------------------
// Display Dimensions
// -----------------------------------------------------------------------------
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
