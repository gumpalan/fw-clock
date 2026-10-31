#include <unity.h>
#include "Format/Format.h"
#include <cstring>
#include <cmath>

using namespace logic;

static void assertLen16(const char* s, const char* label) {
    size_t len = strlen(s);
    char msg[64];
    snprintf(msg, sizeof(msg), "%s: panjang %zu bukan 16", label, len);
    TEST_ASSERT_EQUAL_MESSAGE(16, (int)len, msg);
}

// --- Halaman 0: panjang tepat 16 karakter ---
void test_page0_length() {
    Format fmt(false);
    TimeData t{2026, 10, 31, 14, 32, 5, true};
    SensorData s{27.0f, 61.0f, true};
    char out[2][17];
    fmt.page0(t, s, out);
    assertLen16(out[0], "page0 baris0");
    assertLen16(out[1], "page0 baris1");
}

// --- Halaman 0: NTP belum sync tampilkan "--:--:--" ---
void test_page0_no_sync() {
    Format fmt(false);
    TimeData t{0, 0, 0, 0, 0, 0, false};
    SensorData s{27.0f, 61.0f, true};
    char out[2][17];
    fmt.page0(t, s, out);
    TEST_ASSERT_NOT_NULL(strstr(out[0], "--:--:--"));
    assertLen16(out[0], "page0 no-sync baris0");
}

// --- Halaman 0: sensor gagal tampilkan "--" ---
void test_page0_sensor_fail() {
    Format fmt(false);
    TimeData t{2026, 10, 31, 8, 0, 0, true};
    SensorData s{NAN, NAN, false};
    char out[2][17];
    fmt.page0(t, s, out);
    TEST_ASSERT_NOT_NULL(strstr(out[1], "--"));
    assertLen16(out[1], "page0 sensor-fail baris1");
}

// --- Halaman 1: panjang 16, DHT11 (tanpa desimal) ---
void test_page1_dht11() {
    Format fmt(false);
    SensorData s{27.0f, 61.0f, true};
    char out[2][17];
    fmt.page1(s, out);
    assertLen16(out[0], "page1 dht11 baris0");
    assertLen16(out[1], "page1 dht11 baris1");
    TEST_ASSERT_NOT_NULL(strstr(out[0], "27"));
    TEST_ASSERT_NOT_NULL(strstr(out[1], "61"));
}

// --- Halaman 1: DHT22 (ada desimal) ---
void test_page1_dht22() {
    Format fmt(true);
    SensorData s{27.3f, 61.5f, true};
    char out[2][17];
    fmt.page1(s, out);
    assertLen16(out[0], "page1 dht22 baris0");
    TEST_ASSERT_NOT_NULL(strstr(out[0], "27.3"));
}

// --- Halaman 2: panjang 16 ---
void test_page2_length() {
    Format fmt(false);
    NetData n{true, true, "192.168.1.50"};
    char out[2][17];
    fmt.page2(n, out);
    assertLen16(out[0], "page2 baris0");
    assertLen16(out[1], "page2 baris1");
}

// --- Halaman 2: WiFi/MQTT status tampil benar ---
void test_page2_status() {
    Format fmt(false);
    NetData n{true, false, "10.0.0.5"};
    char out[2][17];
    fmt.page2(n, out);
    TEST_ASSERT_NOT_NULL(strstr(out[0], "OK"));  // WiFi OK
    TEST_ASSERT_NOT_NULL(strstr(out[0], "NO"));  // MQTT NO
}

void run_format_tests() {
    RUN_TEST(test_page0_length);
    RUN_TEST(test_page0_no_sync);
    RUN_TEST(test_page0_sensor_fail);
    RUN_TEST(test_page1_dht11);
    RUN_TEST(test_page1_dht22);
    RUN_TEST(test_page2_length);
    RUN_TEST(test_page2_status);
}
