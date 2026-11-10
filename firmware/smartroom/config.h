#pragma once

// Kredensial: buat secrets.h untuk konfigurasi pribadi.
// Jika file belum ada, konfigurasi default Wokwi akan digunakan.
#include "secrets.h"

// Jika broker memerlukan autentikasi, definisikan MQTT_USER dan MQTT_PASS_
// di secrets.h. Jika tidak, biarkan keduanya tidak didefinisikan.

#define PIN_DHT       4
#define PIN_LDR       34
#define PIN_PIR       27
#define PIN_LED_G     25
#define PIN_LED_R     26
#define PIN_BUZZER    33
#define PIN_BTN       14

#define OLED_W        128
#define OLED_H        64
#define OLED_ADDR     0x3C

constexpr unsigned long READ_INTERVAL_MS   = 5000;
constexpr unsigned long RENDER_INTERVAL_MS = 500;
constexpr unsigned long DEBOUNCE_MS        = 50;
constexpr unsigned long WIFI_RETRY_MS      = 10000;
constexpr unsigned long MQTT_RETRY_MS      = 5000;

constexpr float DEFAULT_THRESHOLD = 30.0f;
constexpr float ALARM_HYSTERESIS  = 0.5f;
constexpr uint8_t SENSOR_FAIL_LIMIT = 3;

constexpr int BUZZER_FREQ = 2000;
constexpr int BUZZER_CH   = 0;
constexpr long GMT_OFFSET_S = 7 * 3600; // WIB
