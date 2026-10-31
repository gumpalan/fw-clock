#pragma once
#include <cstdint>

// =============================================================================
// ButtonClassifier.h — Klasifikasi short/long press dari event bertanda waktu
// Murni C++, tidak bergantung Arduino.h.
// =============================================================================

namespace logic {

enum class PressType : uint8_t {
    NONE  = 0,
    SHORT = 1,
    LONG  = 2,
};

struct BtnEvent {
    uint8_t  pin;
    bool     pressed;   // true = tombol ditekan (LOW), false = dilepas
    uint32_t tick_ms;   // millis() atau tick simulasi
};

struct PressResult {
    uint8_t   pin;
    PressType type;
};

// Satu instance per tombol.
class ButtonClassifier {
public:
    explicit ButtonClassifier(uint8_t pin,
                              uint32_t debounce_ms  = 40,
                              uint32_t long_press_ms = 800);

    // Masukkan event dari ISR / polling. Kembalikan hasil jika press selesai.
    PressResult feed(const BtnEvent& ev);

    // Reset state internal (misal saat sistem restart).
    void reset();

private:
    uint8_t  pin_;
    uint32_t debounce_ms_;
    uint32_t long_press_ms_;

    bool     last_stable_;   // kondisi stabil terakhir
    uint32_t last_change_ms_;
    uint32_t press_start_ms_;
    bool     pending_press_;
};

} // namespace logic
