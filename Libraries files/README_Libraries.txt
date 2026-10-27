Libraries and software dependencies
===================================

1. Arduino IDE 2.x or Visual Studio Code with PlatformIO
2. ESP32 board support package (Espressif Systems)
   Recommended tested target for this source: Arduino-ESP32 core 2.0.17
   https://github.com/espressif/arduino-esp32
3. Built-in ESP32 framework headers used:
   - WiFi.h
   - esp_now.h
   - esp_wifi.h
   - driver/i2s.h
   These are provided by the ESP32 board package; no separate audio library is required.

Hardware note:
This project uses ESP-NOW for low-overhead wireless packet transfer. It is not
Bluetooth. Both boards must use the same Wi-Fi channel. ESP-NOW is chosen here
because it can transfer short audio packets without connecting to a router.

Known limitations to test:
- The ESP-NOW callback API can differ between Arduino-ESP32 core versions.
  If compiling with core 3.x, adapt the receive callback signature to that version.
- Audio quality and latency depend on microphone wiring, Wi-Fi conditions, and
  I2S configuration. This source is a prototype, not a verified hardware test.
- For robust playback, use a ring buffer/FreeRTOS queue between packet reception
  and I2S output, add packet-loss concealment, and tune buffering.
