#include "Gamma.h"
#include <cmath>

namespace logic {

Gamma::Gamma(float gamma, uint32_t duty_max) {
    for (uint8_t i = 0; i < GAMMA_TABLE_SIZE; ++i) {
        float normalized = static_cast<float>(i) / 100.0f;
        float corrected  = powf(normalized, gamma);
        table_[i] = static_cast<uint32_t>(corrected * static_cast<float>(duty_max) + 0.5f);
    }
    // Pastikan batas: 0% = 0, 100% = duty_max
    table_[0]   = 0;
    table_[100] = duty_max;
}

uint32_t Gamma::dutyFromPercent(uint8_t percent) const {
    if (percent >= GAMMA_TABLE_SIZE) percent = 100;
    return table_[percent];
}

uint32_t Gamma::tableAt(uint8_t index) const {
    if (index >= GAMMA_TABLE_SIZE) index = 100;
    return table_[index];
}

} // namespace logic
