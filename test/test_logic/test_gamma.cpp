#include <unity.h>
#include "Gamma/Gamma.h"

using namespace logic;

// --- Batas bawah: 0% = duty 0 ---
void test_gamma_zero() {
    Gamma g(2.2f, 1023);
    TEST_ASSERT_EQUAL_UINT32(0, g.dutyFromPercent(0));
}

// --- Batas atas: 100% = duty 1023 ---
void test_gamma_full() {
    Gamma g(2.2f, 1023);
    TEST_ASSERT_EQUAL_UINT32(1023, g.dutyFromPercent(100));
}

// --- Tabel monoton naik ---
void test_gamma_monotone() {
    Gamma g(2.2f, 1023);
    for (uint8_t i = 1; i < g.tableSize(); ++i) {
        TEST_ASSERT_TRUE_MESSAGE(
            g.tableAt(i) >= g.tableAt(i - 1),
            "Tabel gamma tidak monoton naik"
        );
    }
}

// --- Index di luar range di-clamp ke 100 ---
void test_gamma_clamp() {
    Gamma g(2.2f, 1023);
    TEST_ASSERT_EQUAL_UINT32(g.dutyFromPercent(100), g.dutyFromPercent(200));
}

// --- duty_max berbeda (8-bit: 255) ---
void test_gamma_8bit() {
    Gamma g(2.2f, 255);
    TEST_ASSERT_EQUAL_UINT32(0,   g.dutyFromPercent(0));
    TEST_ASSERT_EQUAL_UINT32(255, g.dutyFromPercent(100));
}

void run_gamma_tests() {
    RUN_TEST(test_gamma_zero);
    RUN_TEST(test_gamma_full);
    RUN_TEST(test_gamma_monotone);
    RUN_TEST(test_gamma_clamp);
    RUN_TEST(test_gamma_8bit);
}
