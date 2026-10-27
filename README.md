# Task 1.1 — Real-Time Voice Transfer Using ESP32

## Objective
Transfer live voice from two microphones connected to a sender ESP32 to a speaker
connected to a second ESP32. The implementation uses ESP-NOW over the ESP32 Wi-Fi
radio to keep packet overhead low and avoid needing a Wi-Fi router.

## Files
- `Main file/Sender.ino` — reads the two I2S microphone channels, mixes them to mono,
  divides audio into small packets, and sends packets using ESP-NOW.
- `Main file/Receiver.ino` — receives packets and outputs audio over I2S.
- `Libraries files/README_Libraries.txt` — dependencies and version notes.
- `Schematic file/system_block_diagram.svg` — system architecture diagram.
- `Schematic file/wiring_and_components.md` — component list and wiring guide.

## Components for a real implementation
- 2 × ESP32 development boards (sender and receiver)
- 2 × INMP441 I2S digital microphone modules
- 1 × MAX98357A I2S class-D amplifier module
- 1 × 4–8 Ω speaker compatible with amplifier and power supply
- USB cables and stable power supplies
- Jumper wires and breadboard/PCB

## Setup and run
1. Install Arduino IDE 2.x and the Espressif ESP32 board package. Select an ESP32
   board and use Arduino-ESP32 core 2.0.17 for the callback/API version used here.
2. Wire the microphones to the sender and the MAX98357A/speaker to the receiver
   according to `Schematic file/wiring_and_components.md`.
3. Upload `Receiver.ino` to the receiver board first. Open Serial Monitor at 115200
   baud and note the receiver station MAC address.
4. Replace the placeholder bytes in `RECEIVER_MAC` in `Sender.ino` with that MAC.
   Example format: `uint8_t RECEIVER_MAC[6] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};`
5. Upload `Sender.ino` to the sender board. Speak close to the microphones and
   listen at the receiver speaker.
6. If there is no audio, verify the MAC address, channel (1 on both boards), wiring,
   module power, and Serial Monitor messages.

## Data flow
Sound → INMP441 microphone pair → I2S digital audio → ESP32 sender → ESP-NOW
packets over 2.4 GHz → ESP32 receiver → I2S → MAX98357A amplifier → speaker.

## Design choices
- Sample rate: 16 kHz, a common rate for voice-band applications.
- Packet payload: 100 mono samples (200 audio bytes plus a 4-byte header), or
  approximately 6.25 ms of audio per packet.
- Two microphone channels are averaged to a mono signal to reduce the data rate.
- A 16-bit sequence number helps identify missing/out-of-order packets during testing.

## Important engineering notes
- This source is intended as a hardware implementation starting point. It has not
  been tested on your physical boards, so do not describe it as hardware-verified.
- Confirm the INMP441 modules' left/right-slot behavior before tying their SD pins
  together. Some modules/boards may require different wiring.
- Real-time audio requires careful buffer management. This prototype writes received
  audio directly to I2S from the ESP-NOW callback for simplicity; a production design
  should use a queue/ring buffer and a separate audio playback task.
- ESP-NOW delivery is not guaranteed. Sequence gaps are reported in Serial Monitor.
- The optional demo video is not included because no hardware recording was supplied.

## Testing checklist
- [ ] Both ESP32 boards power up and appear in Serial Monitor.
- [ ] Receiver MAC is copied correctly into sender code.
- [ ] Both boards use channel 1.
- [ ] Sender receives microphone data and sends packets.
- [ ] Receiver reports incoming packets.
- [ ] Voice can be heard with acceptable latency.
- [ ] Test at different distances and record packet gaps/audio quality.

## External reference links
- ESP32 Arduino core: https://github.com/espressif/arduino-esp32
- ESP-NOW programming guide: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_now.html
- I2S programming guide: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2s.html
- INMP441 datasheet: https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf
- MAX98357A product information: https://www.analog.com/en/products/max98357a.html
