/*
  Task 1.1 - Real-Time Voice Transfer Using ESP32
  Sender board: ESP32 + two INMP441 I2S microphones
  Wireless transport: ESP-NOW (Wi-Fi radio, no router required)

  Target: Arduino-ESP32 core 2.0.17
  Wiring assumes both INMP441 microphones share BCLK and WS, and their
  L/R pins select LEFT and RIGHT slots. Their SD pins are tied together.
  This common arrangement is supported by INMP441 modules that tri-state
  SD outside their selected slot; verify your module's datasheet.

  IMPORTANT:
  1. Replace RECEIVER_MAC with the receiver ESP32's Wi-Fi station MAC.
  2. This is a practical prototype; test with hardware before claiming it works.
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "driver/i2s.h"

static const i2s_port_t MIC_I2S_PORT = I2S_NUM_0;
static const int MIC_BCLK = 26;
static const int MIC_WS   = 25;
static const int MIC_SD   = 33;

static const uint32_t SAMPLE_RATE = 16000;
static const size_t SAMPLES_PER_PACKET = 100; // 6.25 ms at 16 kHz
static const uint8_t ESPNOW_CHANNEL = 1;

// Replace with the receiver board's station MAC address.
uint8_t RECEIVER_MAC[6] = {0x24, 0x6F, 0x28, 0x00, 0x00, 0x00};

struct __attribute__((packed)) AudioPacket {
  uint16_t sequence;
  uint16_t sampleCount;
  int16_t samples[SAMPLES_PER_PACKET];
};
AudioPacket packet;
uint16_t sequenceNumber = 0;

void setupMicrophones() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = false;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins = {};
  pins.bck_io_num = MIC_BCLK;
  pins.ws_io_num = MIC_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = MIC_SD;

  ESP_ERROR_CHECK(i2s_driver_install(MIC_I2S_PORT, &config, 0, nullptr));
  ESP_ERROR_CHECK(i2s_set_pin(MIC_I2S_PORT, &pins));
  ESP_ERROR_CHECK(i2s_zero_dma_buffer(MIC_I2S_PORT));
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    while (true) delay(1000);
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, RECEIVER_MAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Could not add receiver peer. Check receiver MAC/channel.");
    while (true) delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nESP32 voice sender starting...");
  setupMicrophones();
  setupEspNow();
  packet.sampleCount = SAMPLES_PER_PACKET;
  Serial.println("Sender ready. Speak near the microphones.");
}

void loop() {
  // INMP441 supplies 32-bit I2S slots. The useful signed audio data is
  // typically in the upper bits; shifting by 14 gives a 16-bit-scale value.
  int32_t stereo[SAMPLES_PER_PACKET * 2];
  size_t bytesRead = 0;
  const size_t bytesNeeded = sizeof(stereo);

  esp_err_t result = i2s_read(MIC_I2S_PORT, stereo, bytesNeeded,
                              &bytesRead, portMAX_DELAY);
  if (result != ESP_OK || bytesRead != bytesNeeded) return;

  // Mix left/right microphone channels to mono. If only one mic is connected,
  // one channel may be silent; the average still gives a usable signal.
  for (size_t i = 0; i < SAMPLES_PER_PACKET; ++i) {
    int32_t left  = stereo[2 * i] >> 14;
    int32_t right = stereo[2 * i + 1] >> 14;
    int32_t mixed = (left + right) / 2;
    if (mixed > 32767) mixed = 32767;
    if (mixed < -32768) mixed = -32768;
    packet.samples[i] = (int16_t)mixed;
  }

  packet.sequence = sequenceNumber++;
  esp_err_t sent = esp_now_send(RECEIVER_MAC,
                                (uint8_t *)&packet, sizeof(packet));
  if (sent != ESP_OK) {
    Serial.printf("ESP-NOW send error: %d\n", (int)sent);
  }
}
