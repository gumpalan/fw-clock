#include "SensorDht.h"
#include <Arduino.h>

SensorDht::SensorDht(uint8_t pin, uint8_t type, uint32_t interval_ms)
    : dht_(pin, type), interval_ms_(interval_ms), last_read_ms_(0)
{
    reading_.temperature  = NAN;
    reading_.humidity     = NAN;
    reading_.valid        = false;
    reading_.timestamp_ms = 0;
    reading_.fail_streak  = 0;
}

void SensorDht::begin() {
    dht_.begin();
    // Baca pertama kali butuh >= 1 detik setelah begin — biarkan tick() yang lakukan
}

void SensorDht::tick(uint32_t now_ms) {
    if (now_ms - last_read_ms_ < interval_ms_) return;
    last_read_ms_ = now_ms;

    // SATU siklus baca saja — panggil read() sekali, lalu ambil dua nilai dari cache.
    // Memanggil readTemperature() lalu readHumidity() memicu 2 siklus baca;
    // baca kedua sering gagal karena sensor masih sibuk.
    if (!dht_.read()) {
        reading_.valid = false;
        reading_.fail_streak++;
        if (reading_.fail_streak % 12 == 1) {  // log tiap ~30 detik, bukan tiap 2,5 detik
            Serial.printf("[DHT] Baca gagal (streak: %lu)\n",
                          (unsigned long)reading_.fail_streak);
        }
    } else {
        reading_.temperature  = dht_.readTemperature();  // pakai hasil read() tadi
        reading_.humidity     = dht_.readHumidity();
        if (isnan(reading_.temperature) || isnan(reading_.humidity)) {
            reading_.valid = false;
            reading_.fail_streak++;
            Serial.printf("[DHT] Nilai NaN (streak: %lu)\n",
                          (unsigned long)reading_.fail_streak);
        } else {
            reading_.valid        = true;
            reading_.timestamp_ms = now_ms;
            reading_.fail_streak  = 0;
        }
    }
}

bool SensorDht::isFresh(uint32_t now_ms, uint32_t stale_ms) const {
    if (!reading_.valid) return false;
    return (now_ms - reading_.timestamp_ms) < stale_ms;
}
