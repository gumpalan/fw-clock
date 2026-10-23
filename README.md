# fw-clock

Firmware ESP32 untuk jam meja multifungsi dan lampu tidur pintar pada ekosistem Gumpalan.

## Status

**Scaffold** (konfigurasi PlatformIO Arduino framework, template secrets, dan loop minimal).

## Perangkat Keras yang Direncanakan

- ESP32 DevKit V1 (30 atau 38 pin)
- Tampilan jam: modul 4-digit 7-segment TM1637 atau LCD 16x2 I2C (PCF8574)
- Audio alarm/notifikasi: amplifier I2S MAX98357A + speaker mini 3W 4-ohm
- Kontrol lampu tidur: LED strip 12V melalui driver MOSFET (misal modul IRLZ44N / LR7843)
- Tombol kendali: sensor sentuh kapasitif (TTP223 atau pin sentuh bawaan ESP32)
- Opsional: modul RTC DS3231 I2C (pencatat waktu offline presisi) dan modul penerima inframerah (IR receiver)

*Seluruh daftar komponen di atas berstatus rencana rancangan perangkat keras.*

## Fitur yang Direncanakan

1. Penampil waktu otomatis tersinkronisasi via NTP (Network Time Protocol).
2. Alarm audio terprogram yang dapat disetel lewat aplikasi Gumpalan.
3. Lampu tidur dengan pengaturan saklar dan kecerahan (dimmer).
4. Kendali dua arah melalui protokol MQTT lokal (Mosquitto).
5. Pengumuman penemuan otomatis (*auto-discovery*): memublikasikan payload kapabilitas retained ke topik `gumpalan/discovery/<device_id>` saat pertama kali booting.

## Lisensi dan Dependensi

Lisensi firmware ini **belum ditentukan**.

### Lisensi Framework & Komponen:
- Kerangka kerja Arduino-ESP32 berlisensi **LGPL-2.1**. Periksa ketentuan distribusi dan tautan ulang dinamis jika firmware didistribusikan dalam bentuk biner komersial.

### Berkas Rahasia:
Salin `include/secrets.example.h` menjadi `include/secrets.h` dan sesuaikan kredensial WiFi dan broker MQTT lokal Anda. Berkas `secrets.h` diabaikan oleh git dan tidak boleh di-commit.
