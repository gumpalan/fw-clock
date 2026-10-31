#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>

// =============================================================================
// Format.h — Render teks baris LCD dari struct data
// Murni C++, tidak bergantung Arduino.h.
// =============================================================================

namespace logic {

// Data snapshot yang dirender ke LCD.
struct SensorData {
    float    temperature;   // NaN = tidak valid
    float    humidity;      // NaN = tidak valid
    bool     valid;
};

struct TimeData {
    int year, month, day;
    int hour, minute, second;
    bool synced;  // false = NTP belum sinkron
};

struct NetData {
    bool wifi_ok;
    bool mqtt_ok;
    char ip[16];  // "192.168.x.x\0"
};

struct LampData {
    bool    power;
    uint8_t brightness; // 0-100
};

// Halaman LCD 0: "HH:MM:SS  DD/MM" dan "T:27C H:61%"
// Halaman LCD 1: detail sensor
// Halaman LCD 2: status jaringan
// Setiap render mengisi buf[row][17] (16 karakter + null).
class Format {
public:
    // isDht22: tampilkan 1 desimal jika true, tanpa desimal jika false.
    explicit Format(bool isDht22 = false);

    // Render halaman ke out[0] (baris 1) dan out[1] (baris 2).
    // Setiap string tepat 16 karakter (dipadding spasi).
    void page0(const TimeData& t, const SensorData& s,
               char out[2][17]) const;
    void page1(const SensorData& s, char out[2][17]) const;
    void page2(const NetData& n, char out[2][17]) const;

private:
    bool is_dht22_;

    // Format suhu. Tulis ke buf (maks 8 char termasuk null).
    void fmtTemp(float v, char* buf, size_t len) const;
    void fmtHum(float v, char* buf, size_t len) const;

    // Pad atau truncate string ke tepat 16 karakter + null.
    static void pad16(char out[17], const char* src);
};

} // namespace logic
