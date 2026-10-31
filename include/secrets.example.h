#pragma once

// =============================================================================
// secrets.example.h — Template kredensial fw-clock
// Salin ke secrets.h, isi nilai asli, lalu simpan.
// secrets.h TIDAK BOLEH di-commit ke repositori git (sudah masuk .gitignore).
// =============================================================================

#define WIFI_SSID     "NAMA_WIFI_ANDA"
#define WIFI_PASS     "PASSWORD_WIFI_ANDA"

#define MQTT_HOST     "192.168.1.100"
#define MQTT_PORT     1883
#define MQTT_USER     "gumpalan_clock"
#define MQTT_PASS     "PASSWORD_MQTT"

// Opsional: override device ID. Kosongkan (hapus baris ini) untuk auto dari MAC.
// #define DEVICE_ID_OVERRIDE "clock-kamar"
