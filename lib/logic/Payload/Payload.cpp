#include "Payload.h"
#include "../Format/Format.h"
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cmath>

namespace logic {

bool Payload::sensorState(float temperature, float humidity,
                           uint32_t ts, char* out, size_t out_len) {
    JsonDocument doc;
    if (std::isnan(temperature) || std::isnan(humidity)) {
        // Tetap kirim dengan null agar subscriber tahu sensor mati
        doc["temperature"] = nullptr;
        doc["humidity"]    = nullptr;
    } else {
        doc["temperature"] = temperature;
        doc["humidity"]    = humidity;
    }
    doc["ts"] = ts;
    size_t n = serializeJson(doc, out, out_len);
    return n > 0 && n < out_len;
}

bool Payload::lampState(bool power, uint8_t brightness,
                         char* out, size_t out_len) {
    JsonDocument doc;
    doc["power"]      = power ? "on" : "off";
    doc["brightness"] = brightness;
    size_t n = serializeJson(doc, out, out_len);
    return n > 0 && n < out_len;
}

bool Payload::discovery(const char* id, const char* fw_version,
                         char* out, size_t out_len) {
    JsonDocument doc;
    doc["id"]      = id;
    doc["name"]    = "Jam Meja";
    doc["fw"]      = fw_version;

    JsonArray caps = doc["capabilities"].to<JsonArray>();

    // Sensor suhu
    JsonObject temp = caps.add<JsonObject>();
    char topic[80];
    snprintf(topic, sizeof(topic), "gumpalan/%s/sensor/state", id);
    temp["type"]        = "sensor";
    temp["key"]         = "temperature";
    temp["unit"]        = "C";
    temp["state_topic"] = topic;
    temp["value_path"]  = "temperature";

    // Sensor kelembapan
    JsonObject hum = caps.add<JsonObject>();
    hum["type"]        = "sensor";
    hum["key"]         = "humidity";
    hum["unit"]        = "%";
    hum["state_topic"] = topic;  // topik sama dengan suhu
    hum["value_path"]  = "humidity";

    // Lampu switch
    char state_topic[80], cmd_topic[80];
    snprintf(state_topic, sizeof(state_topic), "gumpalan/%s/lamp/state", id);
    snprintf(cmd_topic,   sizeof(cmd_topic),   "gumpalan/%s/lamp/set",   id);

    JsonObject sw = caps.add<JsonObject>();
    sw["type"]          = "switch";
    sw["key"]           = "lamp";
    sw["state_topic"]   = state_topic;
    sw["command_topic"] = cmd_topic;
    sw["value_path"]    = "power";

    // Dimmer
    JsonObject dim = caps.add<JsonObject>();
    dim["type"]          = "dimmer";
    dim["key"]           = "brightness";
    dim["state_topic"]   = state_topic;
    dim["command_topic"] = cmd_topic;
    dim["value_path"]    = "brightness";
    dim["min"]           = 0;
    dim["max"]           = 100;

    size_t n = serializeJson(doc, out, out_len);
    return n > 0 && n < out_len;
}

// Helper: ubah string ke uppercase in-place (maks len karakter)
static void toUpper(char* s, size_t len) {
    for (size_t i = 0; i < len && s[i]; ++i)
        s[i] = static_cast<char>(toupper(static_cast<unsigned char>(s[i])));
}

LampCmd Payload::parseLampCmd(const char* payload, size_t len) {
    LampCmd result{};
    result.valid = false;
    result.field = LampCmd::Field::NONE;

    if (!payload || len == 0) return result;

    // Coba teks polos "ON" / "OFF" dulu
    char plain[8] = {};
    size_t copy = len < 7 ? len : 7;
    memcpy(plain, payload, copy);
    toUpper(plain, copy);

    if (strcmp(plain, "ON") == 0) {
        result.power = true;
        result.brightness = 0; // diabaikan
        result.field = LampCmd::Field::POWER;
        result.valid = true;
        return result;
    }
    if (strcmp(plain, "OFF") == 0) {
        result.power = false;
        result.brightness = 0;
        result.field = LampCmd::Field::POWER;
        result.valid = true;
        return result;
    }

    // Coba parse JSON
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);
    if (err) return result; // payload tidak valid

    bool has_power  = doc["power"].is<const char*>();
    bool has_bright = doc["brightness"].is<int>();

    if (!has_power && !has_bright) return result;

    if (has_power) {
        const char* pv = doc["power"] | "";
        char pvu[8] = {};
        strncpy(pvu, pv, 7);
        toUpper(pvu, 7);
        if (strcmp(pvu, "ON") == 0)       result.power = true;
        else if (strcmp(pvu, "OFF") == 0) result.power = false;
        else return result; // nilai tidak valid
    }

    if (has_bright) {
        int bv = doc["brightness"] | -1;
        if (bv < 0 || bv > 100) return result;
        result.brightness = static_cast<uint8_t>(bv);
    }

    if (has_power && has_bright) result.field = LampCmd::Field::BOTH;
    else if (has_power)          result.field = LampCmd::Field::POWER;
    else                         result.field = LampCmd::Field::BRIGHTNESS;

    result.valid = true;
    return result;
}

} // namespace logic
