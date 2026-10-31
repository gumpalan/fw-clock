#pragma once

// =============================================================================
// pins.h — Peta pin GPIO fw-clock (ESP32 DevKit V1, 30-pin)
// Semua konstanta pin didefinisikan di sini; modul lain hanya include file ini.
// =============================================================================

// --- I2C (LCD PCF8574) -------------------------------------------------------
// Bus ini juga bisa dipakai perangkat I2C lain (RTC, sensor cahaya).
#define PIN_I2C_SDA   21
#define PIN_I2C_SCL   22

// --- DHT sensor --------------------------------------------------------------
// Hindari pin strapping (0, 2, 12, 15) dan pin flash internal (6-11).
#define PIN_DHT       4

// --- LED strip gate (PWM ke MOSFET N-channel) --------------------------------
#define PIN_LED_PWM   18

// --- Tombol (INPUT_PULLUP, aktif LOW) ----------------------------------------
#define PIN_BTN_LAMP    32
#define PIN_BTN_SENSOR  33
#define PIN_BTN_UP      25
#define PIN_BTN_DOWN    26
