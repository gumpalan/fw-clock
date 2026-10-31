#include "TimeService.h"
#include <Arduino.h>
#include <time.h>

TimeService::TimeService(const char* tz,
                         const char* server1,
                         const char* server2,
                         uint32_t    sync_interval_ms)
    : tz_(tz), server1_(server1), server2_(server2),
      sync_interval_ms_(sync_interval_ms),
      last_sync_ms_(0), synced_(false)
{}

void TimeService::begin() {
    doSync();
}

void TimeService::tick(uint32_t now_ms) {
    if (now_ms - last_sync_ms_ >= sync_interval_ms_) {
        doSync();
    }
}

void TimeService::doSync() {
    // configTzTime mengatur timezone dan memulai SNTP
    configTzTime(tz_, server1_, server2_);

    // Tunggu konfirmasi sinkronisasi, maksimal 5 detik non-blocking
    // (kita hanya set flag; verifikasi via now())
    last_sync_ms_ = millis();

    // Cek apakah waktu sudah valid (tahun > 2020)
    struct tm tm_info;
    if (getLocalTime(&tm_info, 0)) {
        if (tm_info.tm_year + 1900 > 2020) {
            synced_ = true;
            Serial.printf("[NTP] Sinkron: %04d-%02d-%02d %02d:%02d:%02d\n",
                          tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
                          tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
        }
    }
}

TimeSnapshot TimeService::now() const {
    TimeSnapshot snap{};
    struct tm tm_info;
    // getLocalTime dengan timeout 0 = non-blocking
    if (!getLocalTime(&tm_info, 0) || tm_info.tm_year + 1900 <= 2020) {
        snap.synced = false;
        return snap;
    }
    snap.year   = tm_info.tm_year + 1900;
    snap.month  = tm_info.tm_mon + 1;
    snap.day    = tm_info.tm_mday;
    snap.hour   = tm_info.tm_hour;
    snap.minute = tm_info.tm_min;
    snap.second = tm_info.tm_sec;
    snap.synced = synced_;
    return snap;
}
