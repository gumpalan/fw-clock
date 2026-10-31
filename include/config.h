#pragma once

// =============================================================================
// config.h — Konstanta tuneable fw-clock
// Ubah nilai di sini sesuai kebutuhan; jangan ubah logika di modul lain.
// =============================================================================

// --- Identitas perangkat -----------------------------------------------------
// Kosongkan untuk auto-generate dari 3 byte terakhir MAC ("clock-XXYYZZ").
// Isi untuk override, misal: #define DEVICE_ID_OVERRIDE "clock-kamar"
// #define DEVICE_ID_OVERRIDE ""

// --- Versi firmware ----------------------------------------------------------
#define FW_VERSION "0.1.0"
#define DEVICE_NAME "Jam Meja"

// --- DHT sensor --------------------------------------------------------------
// Ganti DHT22 jika pakai sensor DHT22 (presisi 0.1 derajat).
#define DHT_TYPE        DHT11
#define DHT_READ_MS     2500UL   // interval baca sensor (ms)
#define DHT_STALE_MS    180000UL // data dianggap stale setelah 3 menit

// --- LCD ---------------------------------------------------------------------
#define LCD_COLS        16
#define LCD_ROWS        2
#define LCD_PAGE_COUNT  3
#define LCD_AUTO_BACK_MS 15000UL // kembali ke halaman 0 setelah idle (ms)

// --- LED strip (PWM) ---------------------------------------------------------
// Versi LEDC API: Arduino-ESP32 3.x (ledcAttach / ledcWrite)
#define LED_FREQ_HZ         5000
#define LED_RESOLUTION_BITS 10        // 0-1023
#define LED_GAMMA           2.2f
#define MAX_LED_DUTY_PERCENT 100      // batas kecerahan (0-100), turunkan jika adaptor lemah
#define LED_FADE_MS         400UL     // durasi transisi (ms)
#define LED_MIN_PERCENT     1         // kecerahan minimum saat ON

// --- Timer tidur (sleep timer) -----------------------------------------------
// 0 = nonaktif. Satuan menit.
#define SLEEP_TIMER_MIN     30

// --- Tombol / Sensor Sentuh --------------------------------------------------
// true  = sensor sentuh TTP223 (aktif HIGH, idle LOW, pinMode INPUT)
// false = tombol mekanis ke GND (aktif LOW, idle HIGH, pinMode INPUT_PULLUP)
#define TOUCH_SENSOR_ACTIVE_HIGH true

#define BTN_DEBOUNCE_MS     40UL
#define BTN_LONG_PRESS_MS   800UL
#define BTN_BRIGHTNESS_STEP 10   // langkah naik/turun kecerahan (%)

// --- MQTT --------------------------------------------------------------------
#define MQTT_BUFFER_SIZE    1024
#define MQTT_SENSOR_PUB_MS  30000UL  // interval publish sensor (ms)
// Backoff reconnect: 1, 2, 4, 8, 16, 30 detik (max)
#define MQTT_BACKOFF_MAX_S  30

// --- NTP ---------------------------------------------------------------------
#define NTP_TZ      "WIB-7"
#define NTP_SERVER1 "id.pool.ntp.org"
#define NTP_SERVER2 "pool.ntp.org"
#define NTP_SYNC_INTERVAL_MS 86400000UL // sinkronisasi ulang tiap 24 jam
