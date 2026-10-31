#pragma once
#include <cstdint>

// =============================================================================
// Gamma.h — Tabel gamma 2.2 dan pemetaan persen ke duty LEDC 10-bit
// Murni C++, tidak bergantung Arduino.h.
// =============================================================================

namespace logic {

// Ukuran tabel: 101 entri (0-100 %).
static constexpr uint8_t GAMMA_TABLE_SIZE = 101;

class Gamma {
public:
    // Bangun tabel dengan gamma dan duty_max (resolusi LEDC).
    // duty_max = (1 << resolution_bits) - 1, misal 1023 untuk 10-bit.
    explicit Gamma(float gamma = 2.2f, uint32_t duty_max = 1023);

    // Petakan persen (0-100) ke nilai duty LEDC.
    uint32_t dutyFromPercent(uint8_t percent) const;

    // Akses tabel (untuk tes monotonisitas).
    uint32_t tableAt(uint8_t index) const;
    uint8_t  tableSize() const { return GAMMA_TABLE_SIZE; }

private:
    uint32_t table_[GAMMA_TABLE_SIZE];
};

} // namespace logic
