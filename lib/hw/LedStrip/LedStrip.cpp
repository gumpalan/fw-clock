#include "LedStrip.h"
#include <Arduino.h>
#include <cmath>

LedStrip::LedStrip(uint8_t pin, uint32_t freq_hz, uint8_t resolution_bits,
                   float gamma, uint8_t max_duty_percent)
    : pin_(pin), freq_hz_(freq_hz), resolution_bits_(resolution_bits),
      max_duty_pct_(max_duty_percent > 100 ? 100 : max_duty_percent),
      on_(false), target_pct_(0), last_on_pct_(50),
      fade_start_ms_(0), fade_duration_ms_(0), duty_start_(0), duty_target_(0)
{
    buildGamma(gamma);
}

void LedStrip::buildGamma(float gamma) {
    uint32_t dmax = dutyMax();
    for (uint8_t i = 0; i <= 100; ++i) {
        float normalized = static_cast<float>(i) / 100.0f;
        float corrected  = powf(normalized, gamma);
        gamma_table_[i]  = static_cast<uint32_t>(corrected * static_cast<float>(dmax) + 0.5f);
    }
    gamma_table_[0]   = 0;
    gamma_table_[100] = dmax;
}

void LedStrip::begin() {
    // Arduino-ESP32 3.x: ledcAttach(pin, freq, resolution)
    ledcAttach(pin_, freq_hz_, resolution_bits_);
    applyDuty(0);
}

void LedStrip::applyDuty(uint32_t duty) {
    // Batas max_duty_pct_
    uint32_t dmax    = dutyMax();
    uint32_t allowed = static_cast<uint32_t>(
        static_cast<float>(dmax) * static_cast<float>(max_duty_pct_) / 100.0f + 0.5f);
    if (duty > allowed) duty = allowed;
    ledcWrite(pin_, duty);
}

void LedStrip::setBrightness(uint8_t percent) {
    if (percent > 100) percent = 100;
    if (percent < 1 && on_) percent = 1; // minimum 1% saat ON

    target_pct_ = percent;
    if (on_ && percent > 0) last_on_pct_ = percent;

    // Mulai fade dari duty saat ini
    uint32_t now_ms = millis();
    // Hitung duty saat ini: interpolasi fade jika sedang berjalan
    uint32_t current_duty;
    if (fading()) {
        uint32_t elapsed = now_ms - fade_start_ms_;
        if (elapsed >= fade_duration_ms_) {
            current_duty = duty_target_;
        } else {
            float t = static_cast<float>(elapsed) / static_cast<float>(fade_duration_ms_);
            current_duty = duty_start_ + static_cast<uint32_t>(
                t * static_cast<float>(static_cast<int32_t>(duty_target_) - static_cast<int32_t>(duty_start_)));
        }
    } else {
        current_duty = gamma_table_[on_ ? target_pct_ : 0];
    }

    duty_start_       = current_duty;
    duty_target_      = gamma_table_[percent];
    fade_start_ms_    = now_ms;
    fade_duration_ms_ = LED_FADE_MS;
}

void LedStrip::setOn(bool on) {
    if (on == on_) return;
    on_ = on;

    if (on) {
        // Pulihkan kecerahan terakhir (minimum 1%)
        uint8_t pct = last_on_pct_;
        if (pct < LED_MIN_PERCENT) pct = LED_MIN_PERCENT;
        target_pct_ = pct;
        setBrightness(pct);
    } else {
        // Fade ke 0
        target_pct_ = 0;
        // Simpan kecerahan sebelum mati
        uint32_t now_ms = millis();
        duty_start_       = gamma_table_[last_on_pct_];
        duty_target_      = 0;
        fade_start_ms_    = now_ms;
        fade_duration_ms_ = LED_FADE_MS;
    }
}

bool LedStrip::fading() const {
    if (fade_duration_ms_ == 0) return false;
    return (millis() - fade_start_ms_) < fade_duration_ms_;
}

void LedStrip::tick(uint32_t now_ms) {
    if (fade_duration_ms_ == 0) return;

    uint32_t elapsed = now_ms - fade_start_ms_;
    if (elapsed >= fade_duration_ms_) {
        applyDuty(duty_target_);
        fade_duration_ms_ = 0; // fade selesai
        return;
    }

    // Interpolasi linear antar duty (gamma sudah diterapkan di duty_start/target)
    float t = static_cast<float>(elapsed) / static_cast<float>(fade_duration_ms_);
    int32_t delta = static_cast<int32_t>(duty_target_) - static_cast<int32_t>(duty_start_);
    uint32_t duty = duty_start_ + static_cast<uint32_t>(t * static_cast<float>(delta));
    applyDuty(duty);
}
