#include "Buttons.h"
#include <Arduino.h>
#include "../../include/config.h"

// Struktur argument ISR per tombol
struct IsrArg {
    QueueHandle_t queue;
    uint8_t       pin_idx;
    uint8_t       gpio_pin;
};

// Array global arg ISR (4 tombol)
static IsrArg isr_args[4];

void IRAM_ATTR Buttons::isrHandler(void* arg) {
    IsrArg* a = static_cast<IsrArg*>(arg);
    BtnRawEvent ev;
    ev.pin_idx = a->pin_idx;

#if TOUCH_SENSOR_ACTIVE_HIGH
    // Sensor sentuh TTP223: disentuh = HIGH
    ev.pressed = (digitalRead(a->gpio_pin) == HIGH);
#else
    // Tombol mekanis ke GND: ditekan = LOW
    ev.pressed = (digitalRead(a->gpio_pin) == LOW);
#endif

    ev.tick_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

    BaseType_t higher_prio = pdFALSE;
    xQueueSendFromISR(a->queue, &ev, &higher_prio);
    if (higher_prio) portYIELD_FROM_ISR();
}

Buttons::Buttons(const uint8_t pins[4],
                 uint32_t debounce_ms,
                 uint32_t long_press_ms)
    : debounce_ms_(debounce_ms), long_press_ms_(long_press_ms), queue_(nullptr)
{
    for (int i = 0; i < 4; ++i) pins_[i] = pins[i];
    for (int i = 0; i < 4; ++i) {
        // stable=false (kondisi awal tidak disentuh / tidak ditekan)
        state_[i] = {false, 0, 0, false};
    }
}

void Buttons::begin() {
    queue_ = xQueueCreate(16, sizeof(BtnRawEvent));

    for (int i = 0; i < 4; ++i) {
#if TOUCH_SENSOR_ACTIVE_HIGH
        pinMode(pins_[i], INPUT);        // sensor sentuh punya push-pull output sendiri
#else
        pinMode(pins_[i], INPUT_PULLUP); // tombol butuh pullup internal
#endif
        isr_args[i].queue    = queue_;
        isr_args[i].pin_idx  = static_cast<uint8_t>(i);
        isr_args[i].gpio_pin = pins_[i];
        attachInterruptArg(digitalPinToInterrupt(pins_[i]),
                           isrHandler,
                           &isr_args[i],
                           CHANGE);
    }
}

bool Buttons::poll(BtnPress& out) {
    if (!queue_) return false;

    BtnRawEvent ev;
    // Ambil satu event dari queue (non-blocking: timeout 0)
    if (xQueueReceive(queue_, &ev, 0) != pdTRUE) return false;

    uint8_t idx = ev.pin_idx;
    if (idx >= 4) return false;

    BtnState& st = state_[idx];

    // Debounce: abaikan jika terlalu cepat dari perubahan terakhir
    if (ev.tick_ms - st.last_change_ms < debounce_ms_) return false;

    bool pressed = ev.pressed;
    if (pressed == st.stable) return false; // tidak ada perubahan state stabil

    st.stable        = pressed;
    st.last_change_ms = ev.tick_ms;

    if (pressed) {
        st.press_start_ms = ev.tick_ms;
        st.pending        = true;
        return false; // tunggu lepas
    }

    if (!st.pending) return false;
    st.pending = false;

    uint32_t duration = ev.tick_ms - st.press_start_ms;
    out.id   = static_cast<BtnId>(idx);
    out.type = (duration >= long_press_ms_) ? PressType::LONG : PressType::SHORT;
    return true;
}
