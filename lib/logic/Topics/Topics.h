#pragma once
#include <cstring>
#include <cstdio>

// =============================================================================
// Topics.h — Builder string topik MQTT dari device id
// Murni C++, tidak bergantung Arduino.h.
// Sensor: topik per channel (angka mentah), bukan JSON gabungan.
// =============================================================================

namespace logic {

// Panjang maksimum satu topik termasuk null.
static constexpr size_t TOPIC_MAX = 80;

class Topics {
public:
    explicit Topics(const char* device_id);

    const char* status()           const { return status_; }
    const char* sensorTemp()       const { return sensor_temp_; }   // publish angka suhu
    const char* sensorHumidity()   const { return sensor_humidity_; } // publish angka RH
    const char* lampState()        const { return lamp_state_; }
    const char* lampSet()          const { return lamp_set_; }
    const char* discovery()        const { return discovery_; }
    const char* deviceId()         const { return device_id_; }

private:
    char device_id_[32];
    char status_[TOPIC_MAX];
    char sensor_temp_[TOPIC_MAX];
    char sensor_humidity_[TOPIC_MAX];
    char lamp_state_[TOPIC_MAX];
    char lamp_set_[TOPIC_MAX];
    char discovery_[TOPIC_MAX];

    void build(char* out, const char* prefix, const char* suffix);
};

} // namespace logic
