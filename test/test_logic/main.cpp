// Test runner Unity untuk env:native
// Semua suite dipanggil dari sini.
#include <unity.h>

// Deklarasi suite dari setiap file tes
void run_button_tests();
void run_gamma_tests();
void run_format_tests();
void run_topics_tests();
void run_payload_tests();

// setUp / tearDown global — setiap file tes juga mendefinisikannya,
// tapi Unity memakai yang ada di translation unit main.
void setUp()    {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    run_button_tests();
    run_gamma_tests();
    run_format_tests();
    run_topics_tests();
    run_payload_tests();
    return UNITY_END();
}
