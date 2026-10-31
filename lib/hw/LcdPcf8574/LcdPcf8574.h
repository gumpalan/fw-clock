#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <cstdint>
#include "../../include/pins.h"
#include "../../include/config.h"

// =============================================================================
// LcdPcf8574.h — Driver LCD 16x2 via backpack PCF8574 (mode 4-bit)
// Ditulis sendiri tanpa library eksternal.
// Pemetaan bit PCF8574: P0=RS, P1=RW, P2=EN, P3=BL, P4-P7=D4-D7
// Kompatibel HD44780 / ST7066U.
// =============================================================================

class LcdPcf8574 {
public:
    // Deteksi otomatis: coba addr_primary dulu, lalu addr_fallback.
    LcdPcf8574(TwoWire& wire = Wire,
               uint8_t addr_primary  = 0x27,
               uint8_t addr_fallback = 0x3F);

    // Inisialisasi I2C dan HD44780. Return false jika LCD tidak terdeteksi.
    bool begin();

    // Cetak teks di posisi (col, row). Dipadding spasi sampai 16 karakter.
    // Hanya tulis karakter yang berubah (cegah flicker).
    void printAt(uint8_t col, uint8_t row, const char* text);

    // Nyalakan / matikan backlight.
    void backlight(bool on);

    // Tulis seluruh dua baris sekaligus (baris0[17], baris1[17]).
    // Hanya baris yang berubah yang dikirim ke LCD.
    void update(const char row0[17], const char row1[17]);

    bool isConnected() const { return connected_; }

private:
    TwoWire& wire_;
    uint8_t  addr_;
    bool     backlight_on_;
    bool     connected_;
    char     cache_[2][17];  // cache baris terakhir untuk diff

    // Tulis byte ke PCF8574. Return false jika I2C error.
    bool writeI2C(uint8_t data);

    // Kirim nibble (4 bit) dengan pulsa EN.
    void sendNibble(uint8_t nibble, bool rs);

    // Kirim byte perintah atau data (2 nibble).
    void sendByte(uint8_t byte, bool rs);

    // Perintah LCD.
    void command(uint8_t cmd);
    void writeChar(uint8_t ch);

    // Pindah kursor ke (col, row).
    void setCursor(uint8_t col, uint8_t row);

    // Scan alamat I2C.
    uint8_t detectAddr(uint8_t primary, uint8_t fallback);
};
