#pragma once
#include <cstdint>
#include <cstring>

// ArduinoJson v7 — tersedia di env:esp32dev dan env:native via lib_deps
#include <ArduinoJson.h>

// =============================================================================
// Payload.h — Builder JSON state/discovery dan parser perintah lamp
// Murni C++, tidak bergantung Arduino.h (hanya ArduinoJson).
// =============================================================================

namespace logic {

struct SensorData;  // forward dari Format.h — definisi lengkap saat compile
struct LampData;

// Hasil parse perintah lamp/set.
struct LampCmd {
    enum class Field : uint8_t { NONE = 0, POWER = 1, BRIGHTNESS = 2, BOTH = 3 };
    bool    power;
    uint8_t brightness; // 0-100
    Field   field;      // field mana yang hadir di payload
    bool    valid;      // false = payload tidak valid / abaikan
};

class Payload {
public:
    // Serialize nilai sensor tunggal ke string angka mentah (tanpa satuan).
    // Sesuai kontrak svc-iot: payload numerik = angka string, bukan JSON bersatuan.
    // out >= 16 byte. Return false jika NaN.
    static bool sensorValue(float value, char* out, size_t out_len);

    // Serialize lamp state ke JSON string. out >= 48 byte.
    static bool lampState(bool power, uint8_t brightness,
                          char* out, size_t out_len);

    // Serialize discovery payload. out >= 768 byte.
    static bool discovery(const char* id, const char* fw_version,
                          char* out, size_t out_len);

    // Parse perintah lamp/set. Terima JSON {"power":..., "brightness":...}
    // atau teks polos "ON"/"OFF". Payload tidak valid -> result.valid = false.
    static LampCmd parseLampCmd(const char* payload, size_t len);
};

} // namespace logic
