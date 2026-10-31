# fw-clock — Jam Meja Pintar Gumpalan

Firmware ESP32 DevKit V1 untuk jam meja multifungsi dengan sensor suhu/kelembapan
dan lampu tidur yang terintegrasi ke ekosistem Gumpalan via MQTT.

## Fitur

- **LCD 16x2 I2C** — tiga halaman: jam+sensor, detail sensor, status jaringan
- **Sensor DHT11/DHT22** — suhu dan kelembapan, baca non-blocking tiap 2,5 detik
- **Waktu NTP** — sinkronisasi via `id.pool.ntp.org`, timezone WIB (UTC+7)
- **Lampu tidur LED strip 12V** — PWM LEDC, tabel gamma 2.2, fade 400ms
- **4 tombol fisik** — kontrol penuh tanpa WiFi/MQTT
- **Sleep timer** — matikan lampu otomatis setelah N menit (default 30)
- **MQTT dua arah** — terima perintah, publish state + discovery ke broker lokal
- **NVS (Preferences)** — ingat kecerahan terakhir saat reboot

---

## Tabel Pin

```
Fungsi              GPIO   Catatan
-----------------   -----  --------------------------------------------------
I2C SDA (LCD)        21    Shared bus; bisa tambah RTC/sensor lain
I2C SCL (LCD)        22
DHT data              4    Tipe DHT11/DHT22 dipilih di config.h
LED strip gate       18    PWM LEDC ch0 ke MOSFET N-channel
Tombol LAMP          32    INPUT_PULLUP, aktif LOW
Tombol SENSOR        33    INPUT_PULLUP, aktif LOW
Tombol UP            25    INPUT_PULLUP, aktif LOW
Tombol DOWN          26    INPUT_PULLUP, aktif LOW
```

Pin yang dihindari: 0, 2, 12, 15 (strapping boot), 6-11 (flash internal).

---

## Diagram Sambungan

### LCD 16x2 I2C (backpack PCF8574)

```
ESP32 3.3V --- [opsional: lepas R pull-up di backpack, pasang 4.7k ke 3.3V]
ESP32 GND  --- LCD GND
ESP32 5V   --- LCD VCC  (dari expand board)
ESP32 GPIO21 - LCD SDA
ESP32 GPIO22 - LCD SCL
```

**Peringatan**: Backpack PCF8574 umumnya memakai resistor pull-up I2C ke 5V.
Ini bisa berjalan di ESP32 (level 3.3V) tetapi tidak sepenuhnya sesuai spesifikasi
karena SDA/SCL bisa tertarik ke 5V saat idle.
Opsi lebih aman: pasang level shifter dua arah, atau lepas resistor pull-up bawaan
backpack dan ganti dengan pull-up 4.7kOhm ke 3.3V.

### MOSFET IRLZ44N untuk LED strip 12V

```
Adaptor 12V (+) ----------- LED strip (+)
Adaptor 12V (-) --+--------- LED strip (-) ke Drain MOSFET
                  |
                  +--------- ESP32 GND   <- GROUND BERSAMA wajib!

ESP32 GPIO18 --- [100 Ohm] --- Gate IRLZ44N
                               Gate --- [10kOhm] --- GND  <- pull-down

IRLZ44N Source ----------- GND (bersama ESP32 dan adaptor)
IRLZ44N Drain  ----------- LED strip (-)
```

**Catatan arus**: LED strip 12V 1A = 12W total. Pastikan adaptor minimal 12V/1A.
Hitung panjang strip: strip 5050 RGB biasanya 14.4W/m, sekitar 0.83m per 1A adaptor.
IRLZ44N tahan 47A drain, lebih dari cukup.

### DHT11

```
ESP32 3.3V --- DHT11 VCC
ESP32 GND  --- DHT11 GND
ESP32 GPIO4 -- DHT11 DATA -- [4.7kOhm pull-up ke 3.3V]
```

### Sensor Sentuh (TTP223)

```
ESP32 3.3V --- VCC TTP223
ESP32 GND  --- GND TTP223
ESP32 GPIO32 - I/O Tombol LAMP   (output aktif HIGH)
ESP32 GPIO33 - I/O Tombol SENSOR (output aktif HIGH)
ESP32 GPIO25 - I/O Tombol UP     (output aktif HIGH)
ESP32 GPIO26 - I/O Tombol DOWN   (output aktif HIGH)
```

Default konfigurasi di `config.h`: `TOUCH_SENSOR_ACTIVE_HIGH true` (pinMode `INPUT`).
Jika menggunakan tombol mekanis ke GND, ubah menjadi `false` (pinMode `INPUT_PULLUP`).

---

## Daya

- **ESP32**: USB atau 5V dari expand board
- **LED strip**: adaptor 12V terpisah
- **Ground bersama wajib**: GND adaptor 12V harus tersambung ke GND ESP32

---

## Cara Build

### 1. Siapkan secrets.h

```sh
cp include/secrets.example.h include/secrets.h
# Edit include/secrets.h, isi WIFI_SSID, WIFI_PASS, MQTT_HOST, dst.
```

### 2. Build

```sh
pio run -e esp32dev
```

### 3. Tes unit (native, tanpa hardware)

```sh
pio test -e native
```

### 4. Flash ke ESP32

```sh
pio run -e esp32dev --target upload
```

### 5. Monitor serial

```sh
pio device monitor --baud 115200
```

---

## Topik MQTT

```
Topik                             Arah       Deskripsi
gumpalan/<id>/status              publish    "online" / "offline" (retained, LWT)
gumpalan/<id>/sensor/state        publish    JSON suhu+kelembapan+ts (setiap 30 detik)
gumpalan/<id>/lamp/state          publish    JSON power+brightness (retained, saat berubah)
gumpalan/<id>/lamp/set            subscribe  Perintah masuk: JSON atau teks ON/OFF
gumpalan/discovery/<id>           publish    Kapabilitas perangkat (retained, saat connect)
```

Contoh payload:

```json
// sensor/state
{"temperature":27.0,"humidity":61,"ts":1730000000}

// lamp/state
{"power":"on","brightness":75}

// lamp/set (diterima)
{"power":"on","brightness":80}
```

Atau teks polos: ON / OFF

---

## Uji MQTT dengan mosquitto

```sh
# Subscribe semua topik perangkat
mosquitto_sub -h 192.168.1.100 -u gumpalan_clock -P PASSWORD -t "gumpalan/#" -v

# Toggle lampu ON
mosquitto_pub -h 192.168.1.100 -u gumpalan_clock -P PASSWORD \
  -t "gumpalan/clock-XXYYZZ/lamp/set" -m "ON"

# Set kecerahan 60%
mosquitto_pub -h 192.168.1.100 -u gumpalan_clock -P PASSWORD \
  -t "gumpalan/clock-XXYYZZ/lamp/set" \
  -m '{"power":"on","brightness":60}'

# Lihat discovery
mosquitto_sub -h 192.168.1.100 -u gumpalan_clock -P PASSWORD \
  -t "gumpalan/discovery/#" -v
```

Ganti clock-XXYYZZ dengan device ID aktual (lihat log serial saat boot).

---

## Fungsi Tombol

```
Tombol    Short press (<800ms)                    Long press (>=800ms)
LAMP      Toggle ON/OFF lampu                     Aktifkan / batalkan sleep timer
SENSOR    Halaman LCD berikutnya                  Publish sensor sekarang
UP        Kecerahan +10% (nyalakan jika mati)     -
DOWN      Kecerahan -10% (minimum 1%)             -
```

LCD kembali otomatis ke halaman 0 setelah 15 detik tanpa tekan tombol.

---

## Discovery MQTT — Usulan v0

Format payload `gumpalan/discovery/<id>` berikut adalah **usulan v0**.
svc-iot akan menyesuaikan skema ini saat integrasi.

```json
{
  "id": "clock-aabbcc",
  "name": "Jam Meja",
  "fw": "0.1.0",
  "capabilities": [
    {
      "type": "sensor", "key": "temperature", "unit": "C",
      "state_topic": "gumpalan/clock-aabbcc/sensor/state",
      "value_path": "temperature"
    },
    {
      "type": "sensor", "key": "humidity", "unit": "%",
      "state_topic": "gumpalan/clock-aabbcc/sensor/state",
      "value_path": "humidity"
    },
    {
      "type": "switch", "key": "lamp",
      "state_topic": "gumpalan/clock-aabbcc/lamp/state",
      "command_topic": "gumpalan/clock-aabbcc/lamp/set",
      "value_path": "power"
    },
    {
      "type": "dimmer", "key": "brightness",
      "state_topic": "gumpalan/clock-aabbcc/lamp/state",
      "command_topic": "gumpalan/clock-aabbcc/lamp/set",
      "value_path": "brightness", "min": 0, "max": 100
    }
  ]
}
```

---

## Lisensi dan Dependensi

Lisensi firmware ini **belum ditentukan**.

```
Library                         Versi    Lisensi
PubSubClient (knolleary)        ^2.8     MIT
DHT sensor library (Adafruit)   ^1.4.6   MIT
Adafruit Unified Sensor         ^1.1.14  Apache-2.0
ArduinoJson (bblanchon)         ^7.1.0   MIT
Unity (ThrowTheSwitch)          ^2.5.2   MIT
framework-arduinoespressif32    3.20017  LGPL-2.1
```

**Catatan LGPL**: Framework Arduino-ESP32 berlisensi LGPL-2.1. Jika firmware ini
didistribusikan dalam bentuk biner komersial, periksa ketentuan linking dinamis LGPL.

**Catatan LEDC API**: Build ini menggunakan Arduino-ESP32 2.x API
(ledcSetup + ledcAttachPin + ledcWrite, channel 0). Jika framework diupgrade ke versi
yang memakai ledcAttach(pin, freq, res), update LedStrip.cpp dan hapus LED_LEDC_CHANNEL.

---

## Rencana (Belum Diimplementasi)

- OTA update — ArduinoOTA atau HTTP OTA via svc-iot
- RTC DS3231 — waktu offline presisi saat WiFi mati; TimeService sudah
  dirancang dengan antarmuka agar mudah ditambah
- Penerima IR — kendali via remote tanpa harus menyentuh tombol
- Sensor cahaya GY-30 (BH1750) — dimming otomatis berdasarkan cahaya ruangan
- Provisioning WiFi — tanpa hardcode SSID (SmartConfig atau captive portal)
- Simpan SSID/password di NVS, bukan di secrets.h

---

## Struktur Proyek

```
fw-clock/
  platformio.ini          env:esp32dev + env:native
  include/
    config.h              Semua konstanta tuneable
    pins.h                Peta pin GPIO
    secrets.example.h     Template kredensial
    secrets.h             Isi sendiri, di-gitignore
  src/main.cpp            Perakitan modul (setup/loop tipis)
  lib/
    logic/                Murni C++, dites di env:native
      ButtonClassifier/
      Gamma/
      Format/
      Topics/
      Payload/
    hw/                   Bergantung Arduino/ESP32
      LcdPcf8574/         Driver LCD PCF8574 mode 4-bit (tulis sendiri)
      LedStrip/           PWM LEDC + fade + gamma
      Buttons/            ISR + FreeRTOS queue + debounce
      SensorDht/          DHT non-blocking
      TimeService/        Abstraksi NTP
      NetService/         WiFi + MQTT + reconnect
      Storage/            NVS via Preferences
  test/test_logic/        30 unit test Unity (semua pass)
```
