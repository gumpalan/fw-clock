#pragma once
#include <Preferences.h>
#include <cstdint>

// =============================================================================
// Storage.h — Penyimpanan NVS via Preferences (kecerahan terakhir)
// Namespace NVS: "fw-clock"
// =============================================================================

class Storage {
public:
    Storage();

    void begin();

    // Simpan kecerahan (0-100).
    void saveBrightness(uint8_t percent);

    // Baca kecerahan. Default 50 jika belum pernah disimpan.
    uint8_t loadBrightness();

private:
    Preferences prefs_;
    static constexpr const char* NS  = "fw-clock";
    static constexpr const char* KEY = "brightness";
};
