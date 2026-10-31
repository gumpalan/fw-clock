#include <unity.h>
#include "ButtonClassifier/ButtonClassifier.h"

using namespace logic;

// --- Short press ---
void test_short_press() {
    ButtonClassifier bc(1, 40, 800);
    // Tekan pada t=100ms
    PressResult r = bc.feed({1, true, 100});
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::NONE), static_cast<int>(r.type));
    // Lepas pada t=300ms (durasi 200ms < 800ms = SHORT)
    r = bc.feed({1, false, 300});
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::SHORT), static_cast<int>(r.type));
}

// --- Long press ---
void test_long_press() {
    ButtonClassifier bc(1, 40, 800);
    // Mulai dari tick 50 agar melewati debounce awal (last_change_ms_=0)
    bc.feed({1, true, 50});
    PressResult r = bc.feed({1, false, 950}); // durasi 900ms >= 800ms = LONG
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::LONG), static_cast<int>(r.type));
}

// --- Debounce: event terlalu cepat diabaikan ---
void test_debounce_ignored() {
    ButtonClassifier bc(1, 40, 800);
    bc.feed({1, true, 0});
    // Lepas hanya 10ms setelah tekan — di bawah debounce 40ms
    PressResult r = bc.feed({1, false, 10});
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::NONE), static_cast<int>(r.type));
}

// --- Pin berbeda diabaikan ---
void test_wrong_pin_ignored() {
    ButtonClassifier bc(1, 40, 800);
    PressResult r = bc.feed({2, true, 100}); // pin 2 bukan pin 1
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::NONE), static_cast<int>(r.type));
}

// --- Reset membersihkan state ---
void test_reset() {
    ButtonClassifier bc(1, 40, 800);
    bc.feed({1, true, 0});
    bc.reset();
    // Setelah reset, lepas tidak menghasilkan press
    PressResult r = bc.feed({1, false, 900});
    TEST_ASSERT_EQUAL(static_cast<int>(PressType::NONE), static_cast<int>(r.type));
}

void run_button_tests() {
    RUN_TEST(test_short_press);
    RUN_TEST(test_long_press);
    RUN_TEST(test_debounce_ignored);
    RUN_TEST(test_wrong_pin_ignored);
    RUN_TEST(test_reset);
}
