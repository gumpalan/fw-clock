#pragma once
#include <Arduino.h>
#include <DHT.h>
#include <cstdint>
#include <cmath>

// =============================================================================
// SensorDht.h — Pembaca DHT non-blocking berbasis millis()
// =============================================================================

struct DhtReading {
    float    temperature;  // NaN = tidak valid
    float    humidity;     // NaN = tidak valid
    bool     valid;
    uint32_t timestamp_ms; // millis() saat baca berhasil
    uint32_t fail_streak;  // jumlah gagal berturut-turut
};

class SensorDht {
public:
    SensorDht(uint8_t pin, uint8_t type, uint32_t interval_ms);

    void begin();

    // Panggil dari loop(). Baca sensor jika sudah waktunya.
    void tick(uint32_t now_ms);

    // Ambil pembacaan terakhir.
    const DhtReading& reading() const { return reading_; }

    // true jika data valid dan belum stale (< stale_ms sejak baca terakhir).
    bool isFresh(uint32_t now_ms, uint32_t stale_ms) const;

private:
    DHT      dht_;
    uint32_t interval_ms_;
    uint32_t last_read_ms_;
    DhtReading reading_;
};
