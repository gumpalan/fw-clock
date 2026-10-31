#include <unity.h>
#include "Payload/Payload.h"
#include <ArduinoJson.h>
#include <cstring>

using namespace logic;

// --- sensorState: JSON valid ---
void test_sensor_state_valid_json() {
    char buf[128];
    bool ok = Payload::sensorState(27.0f, 61.0f, 1730000000UL, buf, sizeof(buf));
    TEST_ASSERT_TRUE(ok);
    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, buf) == DeserializationError::Ok);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 27.0f, doc["temperature"].as<float>());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 61.0f, doc["humidity"].as<float>());
    TEST_ASSERT_EQUAL_UINT32(1730000000UL, doc["ts"].as<uint32_t>());
}

// --- lampState: JSON valid ---
void test_lamp_state_valid_json() {
    char buf[64];
    bool ok = Payload::lampState(true, 75, buf, sizeof(buf));
    TEST_ASSERT_TRUE(ok);
    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, buf) == DeserializationError::Ok);
    TEST_ASSERT_EQUAL_STRING("on", doc["power"].as<const char*>());
    TEST_ASSERT_EQUAL_INT(75, doc["brightness"].as<int>());
}

// --- discovery: JSON valid, berisi 4 kapabilitas ---
void test_discovery_valid_json() {
    char buf[768];
    bool ok = Payload::discovery("clock-aabbcc", "0.1.0", buf, sizeof(buf));
    TEST_ASSERT_TRUE(ok);
    JsonDocument doc;
    TEST_ASSERT_TRUE(deserializeJson(doc, buf) == DeserializationError::Ok);
    TEST_ASSERT_EQUAL_STRING("clock-aabbcc", doc["id"].as<const char*>());
    TEST_ASSERT_EQUAL_INT(4, doc["capabilities"].as<JsonArray>().size());
}

// --- parseLampCmd: teks "ON" ---
void test_parse_plain_on() {
    LampCmd cmd = Payload::parseLampCmd("ON", 2);
    TEST_ASSERT_TRUE(cmd.valid);
    TEST_ASSERT_TRUE(cmd.power);
    TEST_ASSERT_EQUAL(static_cast<int>(LampCmd::Field::POWER),
                      static_cast<int>(cmd.field));
}

// --- parseLampCmd: teks "off" (lowercase) ---
void test_parse_plain_off_lower() {
    LampCmd cmd = Payload::parseLampCmd("off", 3);
    TEST_ASSERT_TRUE(cmd.valid);
    TEST_ASSERT_FALSE(cmd.power);
}

// --- parseLampCmd: JSON {"power":"on","brightness":80} ---
void test_parse_json_both() {
    const char* p = "{\"power\":\"on\",\"brightness\":80}";
    LampCmd cmd = Payload::parseLampCmd(p, strlen(p));
    TEST_ASSERT_TRUE(cmd.valid);
    TEST_ASSERT_TRUE(cmd.power);
    TEST_ASSERT_EQUAL_INT(80, cmd.brightness);
    TEST_ASSERT_EQUAL(static_cast<int>(LampCmd::Field::BOTH),
                      static_cast<int>(cmd.field));
}

// --- parseLampCmd: JSON hanya brightness ---
void test_parse_json_brightness_only() {
    const char* p = "{\"brightness\":50}";
    LampCmd cmd = Payload::parseLampCmd(p, strlen(p));
    TEST_ASSERT_TRUE(cmd.valid);
    TEST_ASSERT_EQUAL(static_cast<int>(LampCmd::Field::BRIGHTNESS),
                      static_cast<int>(cmd.field));
    TEST_ASSERT_EQUAL_INT(50, cmd.brightness);
}

// --- parseLampCmd: payload rusak ---
void test_parse_invalid_payload() {
    const char* p = "GARBAGE{{{{";
    LampCmd cmd = Payload::parseLampCmd(p, strlen(p));
    TEST_ASSERT_FALSE(cmd.valid);
}

// --- parseLampCmd: brightness di luar range ditolak ---
void test_parse_brightness_out_of_range() {
    const char* p = "{\"brightness\":200}";
    LampCmd cmd = Payload::parseLampCmd(p, strlen(p));
    TEST_ASSERT_FALSE(cmd.valid);
}

// --- parseLampCmd: payload kosong ---
void test_parse_empty() {
    LampCmd cmd = Payload::parseLampCmd("", 0);
    TEST_ASSERT_FALSE(cmd.valid);
}

void run_payload_tests() {
    RUN_TEST(test_sensor_state_valid_json);
    RUN_TEST(test_lamp_state_valid_json);
    RUN_TEST(test_discovery_valid_json);
    RUN_TEST(test_parse_plain_on);
    RUN_TEST(test_parse_plain_off_lower);
    RUN_TEST(test_parse_json_both);
    RUN_TEST(test_parse_json_brightness_only);
    RUN_TEST(test_parse_invalid_payload);
    RUN_TEST(test_parse_brightness_out_of_range);
    RUN_TEST(test_parse_empty);
}
