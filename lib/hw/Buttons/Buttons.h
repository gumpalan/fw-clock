#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <cstdint>

// =============================================================================
// Buttons.h — Debounce + klasifikasi tombol via FreeRTOS queue
// ISR hanya push event; debounce & klasifikasi di loop().
// =============================================================================

enum class BtnId : uint8_t {
    LAMP   = 0,
    SENSOR = 1,
    UP     = 2,
    DOWN   = 3,
    COUNT  = 4,
};

enum class PressType : uint8_t {
    SHORT = 0,
    LONG  = 1,
};

struct BtnPress {
    BtnId     id;
    PressType type;
};

// Event mentah dari ISR.
struct BtnRawEvent {
    uint8_t  pin_idx;  // indeks 0-3 sesuai BtnId
    bool     pressed;
    uint32_t tick_ms;
};

class Buttons {
public:
    // pins: array 4 GPIO [LAMP, SENSOR, UP, DOWN]
    Buttons(const uint8_t pins[4],
            uint32_t debounce_ms   = 40,
            uint32_t long_press_ms = 800);

    // Daftarkan ISR dan buat queue. Harus dipanggil di setup().
    void begin();

    // Panggil dari loop(). Proses event dari queue, return press jika ada.
    // Kembalikan false jika queue kosong.
    bool poll(BtnPress& out);

private:
    uint8_t  pins_[4];
    uint32_t debounce_ms_;
    uint32_t long_press_ms_;

    // State per tombol
    struct BtnState {
        bool     stable;
        uint32_t last_change_ms;
        uint32_t press_start_ms;
        bool     pending;
    } state_[4];

    QueueHandle_t queue_;

    // ISR handler — satu fungsi, dibedakan lewat arg.
    static void IRAM_ATTR isrHandler(void* arg);
};
