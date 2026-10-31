#pragma once
#include <Arduino.h>
#include <cstdint>
#include <ctime>

// =============================================================================
// TimeService.h — Abstraksi sumber waktu (NTP via configTzTime)
// Antarmuka sederhana agar nanti bisa ditambah RTC DS3231.
// =============================================================================

struct TimeSnapshot {
    int  year, month, day;
    int  hour, minute, second;
    bool synced;   // true jika NTP sudah berhasil minimal sekali
};

class TimeService {
public:
    TimeService(const char* tz,
                const char* server1,
                const char* server2,
                uint32_t    sync_interval_ms);

    // Panggil sekali di setup() setelah WiFi terhubung.
    void begin();

    // Panggil dari loop(). Cek sinkronisasi ulang jika waktunya.
    void tick(uint32_t now_ms);

    // Ambil snapshot waktu saat ini.
    TimeSnapshot now() const;

    bool isSynced() const { return synced_; }

private:
    const char* tz_;
    const char* server1_;
    const char* server2_;
    uint32_t    sync_interval_ms_;
    uint32_t    last_sync_ms_;
    bool        synced_;

    void doSync();
};
