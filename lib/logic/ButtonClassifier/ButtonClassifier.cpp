#include "ButtonClassifier.h"

namespace logic {

ButtonClassifier::ButtonClassifier(uint8_t pin,
                                   uint32_t debounce_ms,
                                   uint32_t long_press_ms)
    : pin_(pin),
      debounce_ms_(debounce_ms),
      long_press_ms_(long_press_ms),
      last_stable_(false),
      last_change_ms_(0),
      press_start_ms_(0),
      pending_press_(false)
{}

PressResult ButtonClassifier::feed(const BtnEvent& ev) {
    PressResult result{pin_, PressType::NONE};

    if (ev.pin != pin_) return result;

    // Debounce: abaikan perubahan sebelum debounce_ms berlalu
    if (ev.tick_ms - last_change_ms_ < debounce_ms_) return result;

    bool pressed = ev.pressed;
    if (pressed == last_stable_) return result;

    last_stable_   = pressed;
    last_change_ms_ = ev.tick_ms;

    if (pressed) {
        // Tombol ditekan — catat waktu mulai
        press_start_ms_ = ev.tick_ms;
        pending_press_  = true;
    } else if (pending_press_) {
        // Tombol dilepas — tentukan tipe press
        pending_press_ = false;
        uint32_t duration = ev.tick_ms - press_start_ms_;
        result.type = (duration >= long_press_ms_) ? PressType::LONG : PressType::SHORT;
    }

    return result;
}

void ButtonClassifier::reset() {
    last_stable_    = false;
    last_change_ms_ = 0;
    press_start_ms_ = 0;
    pending_press_  = false;
}

} // namespace logic
