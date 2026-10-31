#include "Format.h"
#include <cmath>
#include <cstring>
#include <cstdio>

namespace logic {

Format::Format(bool isDht22) : is_dht22_(isDht22) {}

// Pad atau truncate src ke tepat 16 karakter, null-terminated di [16].
void Format::pad16(char out[17], const char* src) {
    size_t len = strlen(src);
    if (len > 16) len = 16;
    memcpy(out, src, len);
    for (size_t i = len; i < 16; ++i) out[i] = ' ';
    out[16] = '\0';
}

void Format::fmtTemp(float v, char* buf, size_t len) const {
    if (std::isnan(v)) {
        snprintf(buf, len, "--");
        return;
    }
    if (is_dht22_) snprintf(buf, len, "%.1f", v);
    else           snprintf(buf, len, "%d", static_cast<int>(v));
}

void Format::fmtHum(float v, char* buf, size_t len) const {
    if (std::isnan(v)) {
        snprintf(buf, len, "--");
        return;
    }
    if (is_dht22_) snprintf(buf, len, "%.1f", v);
    else           snprintf(buf, len, "%d", static_cast<int>(v));
}

// Halaman 0: baris1="HH:MM:SS  DD/MM"  baris2="T:27C H:61%"
void Format::page0(const TimeData& t, const SensorData& s,
                   char out[2][17]) const {
    char tmp[32];

    // Baris 1 — waktu + tanggal
    if (!t.synced) {
        snprintf(tmp, sizeof(tmp), "--:--:--  --/--");
    } else {
        snprintf(tmp, sizeof(tmp), "%02d:%02d:%02d  %02d/%02d",
                 t.hour, t.minute, t.second, t.day, t.month);
    }
    pad16(out[0], tmp);

    // Baris 2 — sensor ringkas
    char tbuf[8], hbuf[8];
    fmtTemp(s.temperature, tbuf, sizeof(tbuf));
    fmtHum(s.humidity, hbuf, sizeof(hbuf));
    // "T:27C H:61%" atau "T:--C H:--%" — max 16 char
    snprintf(tmp, sizeof(tmp), "T:%sC H:%s%%", tbuf, hbuf);
    pad16(out[1], tmp);
}

// Halaman 1: detail sensor
void Format::page1(const SensorData& s, char out[2][17]) const {
    char tmp[32];
    char tbuf[8], hbuf[8];
    fmtTemp(s.temperature, tbuf, sizeof(tbuf));
    fmtHum(s.humidity, hbuf, sizeof(hbuf));

    // Baris 1: "Suhu   27 C" atau "Suhu   27.0 C"
    snprintf(tmp, sizeof(tmp), "Suhu   %s C", tbuf);
    pad16(out[0], tmp);

    // Baris 2: "Lembab 61 %" atau "Lembab 61.0 %"
    snprintf(tmp, sizeof(tmp), "Lembab %s %%", hbuf);
    pad16(out[1], tmp);
}

// Halaman 2: status jaringan
void Format::page2(const NetData& n, char out[2][17]) const {
    char tmp[32];

    // Baris 1: "WiFi:OK MQTT:OK" atau "WiFi:NO MQTT:NO"
    snprintf(tmp, sizeof(tmp), "WiFi:%-2s MQTT:%-2s",
             n.wifi_ok ? "OK" : "NO",
             n.mqtt_ok ? "OK" : "NO");
    pad16(out[0], tmp);

    // Baris 2: IP address (maks 15 karakter)
    pad16(out[1], n.ip[0] ? n.ip : "---");
}

} // namespace logic
