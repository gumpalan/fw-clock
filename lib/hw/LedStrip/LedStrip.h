#pragma once
#include <Arduino.h>
#include <cstdint>
#include "../../include/config.h"

// =============================================================================
// LedStrip.h — Kontrol LED strip 12V via PWM LEDC (Arduino-ESP32 2.x)
// LEDC API 2.x: ledcSetup(channel, freq, res) + ledcAttachPin(pin, ch) + ledcWrite(ch, duty)
// =============================================================================

// Channel LEDC yang dipakai (0-15, pilih yang tidak konflik dengan lib lain)
#define LED_LEDC_CHANNEL 0

class LedStrip {
public:
    LedStrip(uint8_t pin, uint32_t freq_hz, uint8_t resolution_bits,
             float gamma, uint8_t max_duty_percent);

    void begin();

    // Set kecerahan target (0-100%). Fade non-blocking via tick().
    void setBrightness(uint8_t percent);

    // Toggle ON/OFF. ON memulihkan kecerahan terakhir.
    void setOn(bool on);
    bool isOn()         const { return on_; }
    uint8_t brightness()const { return target_pct_; }

    // Harus dipanggil dari loop() setiap iterasi — mengelola fade.
    void tick(uint32_t now_ms);

    // Apakah sedang dalam proses fade?
    bool fading() const;

private:
    uint8_t  pin_;
    uint32_t freq_hz_;
    uint8_t  resolution_bits_;
    uint8_t  max_duty_pct_;
    bool     on_;

    uint8_t  target_pct_;   // target kecerahan (0-100)
    uint8_t  last_on_pct_;  // kecerahan terakhir saat ON (untuk restore)

    uint32_t fade_start_ms_;
    uint32_t fade_duration_ms_;
    uint32_t duty_start_;   // duty saat fade dimulai
    uint32_t duty_target_;  // duty target

    // Tabel gamma (0-100 -> duty)
    uint32_t gamma_table_[101];
    void     buildGamma(float gamma);
    uint32_t dutyMax() const { return (1u << resolution_bits_) - 1; }

    void applyDuty(uint32_t duty);
};
