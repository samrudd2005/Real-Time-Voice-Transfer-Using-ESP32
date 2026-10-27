/*
  Task 1.1 - Real-Time Voice Transfer Using ESP32
  Receiver board: ESP32 + MAX98357A I2S amplifier + speaker
  Target: Arduino-ESP32 core 2.0.17

  The receiver prints its station MAC at startup. Copy that MAC into
  RECEIVER_MAC in Sender.ino, then upload Sender.ino to the sender board.
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "driver/i2s.h"

static const i2s_port_t SPEAKER_I2S_PORT = I2S_NUM_0;
static const int SPK_BCLK = 26;
static const int SPK_LRC  = 25;
static const int SPK_DIN  = 22;
static const uint32_t SAMPLE_RATE = 16000;
static const uint8_t ESPNOW_CHANNEL = 1;
static const size_t SAMPLES_PER_PACKET = 100;

struct __attribute__((packed)) AudioPacket {
  uint16_t sequence;
  uint16_t sampleCount;
  int16_t samples[SAMPLES_PER_PACKET];
};

// Callback runs when a packet arrives. Keep it short: copy audio into a queue
// would be best for production; this simple version writes samples directly.
volatile uint32_t receivedPackets = 0;
volatile uint32_t droppedSequencePackets = 0;
uint16_t expectedSequence = 0;

void setupSpeaker() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = true;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins = {};
  pins.bck_io_num = SPK_BCLK;
  pins.ws_io_num = SPK_LRC;
  pins.data_out_num = SPK_DIN;
  pins.data_in_num = I2S_PIN_NO_CHANGE;

  ESP_ERROR_CHECK(i2s_driver_install(SPEAKER_I2S_PORT, &config, 0, nullptr));
  ESP_ERROR_CHECK(i2s_set_pin(SPEAKER_I2S_PORT, &pins));
  ESP_ERROR_CHECK(i2s_zero_dma_buffer(SPEAKER_I2S_PORT));
}

void onDataReceived(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != (int)sizeof(AudioPacket)) return;
  AudioPacket packet;
  memcpy(&packet, incomingData, sizeof(packet));
  if (packet.sampleCount != SAMPLES_PER_PACKET) return;

  if (packet.sequence != expectedSequence) {
    droppedSequencePackets++;
  }
  expectedSequence = packet.sequence + 1;

  // I2S output is configured as stereo. Duplicate mono samples into L/R slots.
  int16_t stereo[SAMPLES_PER_PACKET * 2];
  for (size_t i = 0; i < SAMPLES_PER_PACKET; ++i) {
    stereo[2 * i] = packet.samples[i];
    stereo[2 * i + 1] = packet.samples[i];
  }
  size_t bytesWritten = 0;
  i2s_write(SPEAKER_I2S_PORT, stereo, sizeof(stereo),
            &bytesWritten, 0);
  receivedPackets++;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.print("Receiver station MAC: ");
  Serial.println(WiFi.macAddress());
  setupSpeaker();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    while (true) delay(1000);
  }
  esp_now_register_recv_cb(onDataReceived);
  Serial.println("Receiver ready. Connect speaker amplifier and run sender.");
}

void loop() {
  static uint32_t lastReport = 0;
  if (millis() - lastReport >= 3000) {
    lastReport = millis();
    Serial.printf("Packets received: %lu | sequence gaps: %lu\n",
                  (unsigned long)receivedPackets,
                  (unsigned long)droppedSequencePackets);
  }
  delay(5);
}
