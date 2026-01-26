# Sourdough Incubator 🍞

Real-time monitoring system for sourdough fermentation using ESP32 microcontrollers.

## Overview

A WiFi-enabled incubator that monitors temperature, humidity, and sourdough level via wireless sensors. Features a web interface, animated visualization, and email notifications for optimal fermentation tracking.

## Features

- **Real-time Monitoring**: Temperature, humidity, and sourdough level via ESP-NOW wireless protocol
- **Web Dashboard**: Mobile-friendly interface at `http://zakwas.local` or `192.168.4.1`
- **Email Notifications**: Test email sending with Gmail SMTP
- **Dual Display**:
  - ST7789 LCD (240x240): Live sensor readings
  - SSD1306 OLED (128x64): Animated fermentation visualization
- **mDNS Support**: Easy access without knowing device IP
- **WiFi Configuration**: Save home WiFi credentials for email functionality
- **Persistent Storage**: Settings saved in non-volatile memory

## Hardware

**Main Module (this project)**:
- ESP32 (uPesy DevKit)
- ST7789 LCD Display (240x240)
- SSD1306 OLED Display (128x64)
- Hardware SPI & I2C connections

**Remote Sensor Module** (separate project):
- ESP32 with DHT22 (temp/humidity) + ultrasonic sensor (level)
- ESP-NOW sender on channel 1

## Getting Started

### Prerequisites
- PlatformIO CLI or VS Code PlatformIO extension
- Python 3.8+

### Setup

1. **Clone and configure**:
   ```bash
   cd "MAIN APP/main-module"
   ```

2. **Update settings** (if needed):
   - Edit `src/config/settings.h` for WiFi AP name, email credentials
   - Edit `src/config/pins.h` for display pin assignments

3. **Build and upload**:
   ```bash
   pio run --target upload --environment upesy_wroom
   ```

4. **Connect and configure**:
   - Join AP "Zakwas-Chlebowy" (password in settings.h)
   - Open `http://zakwas.local`
   - Configure home WiFi and email recipient

## Usage

### Web Interface
- **🥖 Sourdough Tab**: Fermentation state and mini sensor readings
- **⚙️ Settings Tab**:
  - WiFi credentials (for email functionality)
  - Email recipient address
  - Test email button

### Display Output
- **LCD**: Live temperature, humidity, level (updates continuously)
- **OLED**: Animated jar with rising bubbles (fermentation activity)

### Email Notifications
1. Save home WiFi SSID and password in web settings
2. Enter recipient email address
3. Click "Send Test Email" to verify connectivity
4. System disconnects from WiFi and restores ESP-NOW after sending

## Architecture

```
src/
├── config/          # PIN and application settings
├── data/            # Sensor data structures
├── display/         # LCD and OLED drivers
├── comm/            # ESP-NOW receiver
├── web/             # HTTP server and REST API
└── main.cpp         # Application entry point
```

## Technical Notes

- **WiFi Mode**: AP+STA (both simultaneously for web server and email)
- **ESP-NOW Channel**: Fixed to channel 1 (must match sensor module)
- **Email Sequence**: Deinitializes ESP-NOW before WiFi connection, restores after
- **Storage**: Preferences library for non-volatile settings
- **Optimization**: 89% Flash usage, ~15% RAM

## API Endpoints

- `GET /api/data` - Sensor readings and system status
- `GET /api/wifi` - Saved WiFi SSID
- `POST /api/wifi` - Save WiFi credentials
- `POST /api/wifi/clear` - Clear WiFi settings
- `GET /api/email` - Email recipient
- `POST /api/email` - Save recipient and send test email

## Troubleshooting

**No sensor data?**
- Verify sensor module is powered and on channel 1
- Check MAC address printed on Serial output

**Email won't send?**
- Confirm home WiFi SSID/password saved correctly
- Check app password (not regular Gmail password)
- Verify recipient email address format

**Web interface not responding?**
- Restart device
- Reconnect to "Zakwas-Chlebowy" AP
- Try `192.168.4.1` instead of `zakwas.local`

## License

MIT

---

**Designed for hobbyist sourdough bakers** 🧑‍🍳
