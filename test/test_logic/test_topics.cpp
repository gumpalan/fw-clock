#include <unity.h>
#include "Topics/Topics.h"
#include <cstring>

using namespace logic;

void test_topics_prefix() {
    Topics t("clock-aabbcc");
    TEST_ASSERT_EQUAL_STRING("gumpalan/clock-aabbcc/status",       t.status());
    TEST_ASSERT_EQUAL_STRING("gumpalan/clock-aabbcc/sensor/state", t.sensorState());
    TEST_ASSERT_EQUAL_STRING("gumpalan/clock-aabbcc/lamp/state",   t.lampState());
    TEST_ASSERT_EQUAL_STRING("gumpalan/clock-aabbcc/lamp/set",     t.lampSet());
    TEST_ASSERT_EQUAL_STRING("gumpalan/discovery/clock-aabbcc",    t.discovery());
}

void test_topics_device_id() {
    Topics t("clock-112233");
    TEST_ASSERT_EQUAL_STRING("clock-112233", t.deviceId());
}

// Tidak ada "godho" di topik manapun
void test_no_godho_prefix() {
    Topics t("clock-aabbcc");
    TEST_ASSERT_NULL(strstr(t.status(),      "godho"));
    TEST_ASSERT_NULL(strstr(t.sensorState(), "godho"));
    TEST_ASSERT_NULL(strstr(t.lampState(),   "godho"));
    TEST_ASSERT_NULL(strstr(t.lampSet(),     "godho"));
    TEST_ASSERT_NULL(strstr(t.discovery(),   "godho"));
}

void run_topics_tests() {
    RUN_TEST(test_topics_prefix);
    RUN_TEST(test_topics_device_id);
    RUN_TEST(test_no_godho_prefix);
}
