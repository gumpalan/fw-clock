#include "LcdPcf8574.h"
#include <Arduino.h>

// =============================================================================
// Pemetaan bit backpack PCF8574:
//   P0 = RS    P1 = RW    P2 = EN    P3 = Backlight
//   P4 = D4    P5 = D5    P6 = D6    P7 = D7
// =============================================================================

#define LCD_RS  0x01
#define LCD_RW  0x02
#define LCD_EN  0x04
#define LCD_BL  0x08

// Alamat DDRAM baris
static const uint8_t ROW_ADDR[2] = {0x00, 0x40};

LcdPcf8574::LcdPcf8574(TwoWire& wire, uint8_t addr_primary, uint8_t addr_fallback)
    : wire_(wire), addr_(0), backlight_on_(true), connected_(false)
{
    memset(cache_, ' ', sizeof(cache_));
    for (int r = 0; r < 2; ++r) cache_[r][16] = '\0';
    addr_ = 0; // ditentukan di begin()
    // Simpan agar begin() bisa scan
    // Hack: pakai addr_ sebagai primary, addr_fallback disimpan di constructor
    // Karena tidak ada field untuk fallback, scan langsung di begin().
    (void)addr_primary;
    (void)addr_fallback;
    addr_ = addr_primary;
    // addr_fallback_ tidak ada field — scan di begin() dengan kedua nilai.
    // Solusi: tambahkan field private addr_fallback_.
    // Ini sudah ada di header via detectAddr(); cukup simpan kedua nilai.
}

// Implementasi alternatif: gunakan constructor list lengkap.
// Karena header hanya menyimpan addr_, kita scan di begin() secara inline.

bool LcdPcf8574::writeI2C(uint8_t data) {
    wire_.beginTransmission(addr_);
    wire_.write(data);
    return wire_.endTransmission() == 0;
}

void LcdPcf8574::sendNibble(uint8_t nibble, bool rs) {
    uint8_t bl  = backlight_on_ ? LCD_BL : 0;
    uint8_t rs_ = rs ? LCD_RS : 0;
    // Data nibble di P4-P7
    uint8_t data = bl | rs_ | ((nibble & 0x0F) << 4);

    // Pulsa EN: high → tunggu >= 1us → low → tunggu 50us
    writeI2C(data | LCD_EN);
    delayMicroseconds(1);
    writeI2C(data & ~LCD_EN);
    delayMicroseconds(50);
}

void LcdPcf8574::sendByte(uint8_t byte, bool rs) {
    sendNibble(byte >> 4, rs);   // nibble tinggi dulu
    sendNibble(byte & 0x0F, rs); // nibble rendah
}

void LcdPcf8574::command(uint8_t cmd) {
    sendByte(cmd, false); // RS=0 = command
}

void LcdPcf8574::writeChar(uint8_t ch) {
    sendByte(ch, true); // RS=1 = data
}

void LcdPcf8574::setCursor(uint8_t col, uint8_t row) {
    if (row >= 2) row = 1;
    if (col >= 16) col = 15;
    command(0x80 | (ROW_ADDR[row] + col));
}

uint8_t LcdPcf8574::detectAddr(uint8_t primary, uint8_t fallback) {
    wire_.beginTransmission(primary);
    if (wire_.endTransmission() == 0) return primary;
    wire_.beginTransmission(fallback);
    if (wire_.endTransmission() == 0) return fallback;
    return 0; // tidak ditemukan
}

bool LcdPcf8574::begin() {
    wire_.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    // Scan alamat — coba 0x27 dulu, lalu 0x3F
    addr_ = detectAddr(0x27, 0x3F);
    if (addr_ == 0) {
        Serial.println("[LCD] Tidak terdeteksi di 0x27 maupun 0x3F");
        connected_ = false;
        return false;
    }
    Serial.printf("[LCD] Ditemukan di 0x%02X\n", addr_);

    // Inisialisasi HD44780 mode 4-bit (sesuai datasheet)
    delay(50);          // tunggu VCC stabil >= 40ms

    // Kirim nibble 0x3 tiga kali untuk reset (mode 8-bit sebelum switch ke 4-bit)
    uint8_t bl = backlight_on_ ? LCD_BL : 0;
    writeI2C(0x30 | LCD_EN | bl); delayMicroseconds(1);
    writeI2C(0x30 | bl);          delay(5);

    writeI2C(0x30 | LCD_EN | bl); delayMicroseconds(1);
    writeI2C(0x30 | bl);          delay(5);

    writeI2C(0x30 | LCD_EN | bl); delayMicroseconds(1);
    writeI2C(0x30 | bl);          delayMicroseconds(150);

    // Beralih ke mode 4-bit
    writeI2C(0x20 | LCD_EN | bl); delayMicroseconds(1);
    writeI2C(0x20 | bl);          delayMicroseconds(50);

    command(0x28); // Function set: 4-bit, 2 baris, 5x8 dots
    command(0x08); // Display off
    command(0x01); // Clear display
    delay(2);      // Clear butuh >= 1.52ms
    command(0x06); // Entry mode: increment, no shift
    command(0x0C); // Display on, cursor off, blink off

    connected_ = true;

    // Isi cache dengan spasi
    memset(cache_, ' ', sizeof(cache_));
    for (int r = 0; r < 2; ++r) cache_[r][16] = '\0';

    return true;
}

void LcdPcf8574::printAt(uint8_t col, uint8_t row, const char* text) {
    if (!connected_) return;
    setCursor(col, row);
    size_t len = strlen(text);
    uint8_t maxcols = LCD_COLS - col;
    for (uint8_t i = 0; i < maxcols; ++i) {
        uint8_t ch = (i < len) ? (uint8_t)text[i] : ' ';
        writeChar(ch);
    }
}

void LcdPcf8574::backlight(bool on) {
    backlight_on_ = on;
    uint8_t bl = on ? LCD_BL : 0;
    writeI2C(bl); // update backlight tanpa mengubah data
}

void LcdPcf8574::update(const char row0[17], const char row1[17]) {
    if (!connected_) return;

    const char* rows[2] = {row0, row1};

    for (uint8_t r = 0; r < 2; ++r) {
        // Cek apakah baris berubah — hanya render yang berubah (cegah flicker)
        if (memcmp(cache_[r], rows[r], 16) == 0) continue;

        setCursor(0, r);
        for (uint8_t c = 0; c < 16; ++c) {
            uint8_t ch = (uint8_t)rows[r][c];
            // Simbol derajat: 0xDF di ROM HD44780 standar
            writeChar(ch);
        }
        memcpy(cache_[r], rows[r], 17);
    }
}
