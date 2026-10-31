// =============================================================================
// main.cpp — fw-clock (Gumpalan)
// Perakitan modul: setup/loop tipis, semua logika di modul masing-masing.
// =============================================================================

#include <Arduino.h>
#include <WiFi.h>

// Konfigurasi & pin
#include "config.h"
#include "pins.h"
#include "secrets.h"   // WIFI_SSID, WIFI_PASS, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS

// Modul hardware
#include "LcdPcf8574/LcdPcf8574.h"
#include "SensorDht/SensorDht.h"
#include "TimeService/TimeService.h"
#include "LedStrip/LedStrip.h"
#include "Storage/Storage.h"
#include "Buttons/Buttons.h"
#include "NetService/NetService.h"

// Modul logic
#include "ButtonClassifier/ButtonClassifier.h"
#include "Format/Format.h"
#include "Topics/Topics.h"
#include "Payload/Payload.h"

// =============================================================================
// Device ID
// =============================================================================
static char device_id[32];

static void buildDeviceId() {
#ifdef DEVICE_ID_OVERRIDE
    strncpy(device_id, DEVICE_ID_OVERRIDE, sizeof(device_id) - 1);
#else
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(device_id, sizeof(device_id), "clock-%02x%02x%02x",
             mac[3], mac[4], mac[5]);
#endif
}

// =============================================================================
// Forward declarations
// =============================================================================
static void onLampCmd(const char* payload, size_t len);
static void applyLampState(bool power, uint8_t brightness, bool publish_now);

// =============================================================================
// Instance modul
// =============================================================================
static LcdPcf8574 lcd;
static SensorDht  sensor(PIN_DHT, DHT_TYPE, DHT_READ_MS);
static TimeService timesvc(NTP_TZ, NTP_SERVER1, NTP_SERVER2, NTP_SYNC_INTERVAL_MS);
static LedStrip   led(PIN_LED_PWM, LED_FREQ_HZ, LED_RESOLUTION_BITS, LED_GAMMA, MAX_LED_DUTY_PERCENT);
static Storage    storage;

static const uint8_t BTN_PINS[4] = {
    PIN_BTN_LAMP, PIN_BTN_SENSOR, PIN_BTN_UP, PIN_BTN_DOWN
};
static Buttons buttons(BTN_PINS, BTN_DEBOUNCE_MS, BTN_LONG_PRESS_MS);

// Topics & Format (dibuat setelah device_id diketahui)
static logic::Topics*  topics    = nullptr;
static logic::Format*  formatter = nullptr;

// NetService (dibuat setelah Topics)
static NetService* net = nullptr;

// Discovery payload (besar, alokasi di heap)
static char discovery_buf[768];

// =============================================================================
// State aplikasi
// =============================================================================
static uint8_t  lcd_page            = 0;
static uint32_t last_page_change_ms = 0;

static bool     sleep_timer_active  = false;
static uint32_t sleep_timer_end_ms  = 0;

static uint32_t last_sensor_pub_ms  = 0;

// Pesan sementara di baris 2 (non-blocking pengganti delay(1000))
static char     lcd_msg[17]         = {};
static uint32_t lcd_msg_end_ms      = 0;

static char lcd_row[2][17];

// =============================================================================
// Render LCD berdasarkan halaman aktif
// =============================================================================
static void renderLcd(uint32_t now_ms) {
    logic::TimeData td;
    TimeSnapshot ts = timesvc.now();
    td.year    = ts.year; td.month  = ts.month;  td.day    = ts.day;
    td.hour    = ts.hour; td.minute = ts.minute; td.second = ts.second;
    td.synced  = ts.synced;

    const DhtReading& rd = sensor.reading();
    logic::SensorData sd;
    bool fresh = sensor.isFresh(now_ms, DHT_STALE_MS);
    sd.valid       = fresh;
    sd.temperature = fresh ? rd.temperature : NAN;
    sd.humidity    = fresh ? rd.humidity    : NAN;

    switch (lcd_page) {
        case 0: formatter->page0(td, sd, lcd_row); break;
        case 1: formatter->page1(sd, lcd_row);     break;
        case 2: {
            logic::NetData nd;
            nd.wifi_ok = net ? net->wifiOk() : false;
            nd.mqtt_ok = net ? net->mqttOk() : false;
            if (net) net->ipStr(nd.ip);
            else     strncpy(nd.ip, "---", sizeof(nd.ip));
            formatter->page2(nd, lcd_row);
            break;
        }
        default: lcd_page = 0; formatter->page0(td, sd, lcd_row); break;
    }

    // Pesan sementara (dari tombol): override baris 2 selama 1 detik, non-blocking
    if (lcd_msg_end_ms != 0) {
        if (now_ms >= lcd_msg_end_ms) {
            lcd_msg[0]     = '\0';
            lcd_msg_end_ms = 0;
        } else {
            size_t len = strlen(lcd_msg);
            if (len > 16) len = 16;
            memcpy(lcd_row[1], lcd_msg, len);
            for (size_t i = len; i < 16; ++i) lcd_row[1][i] = ' ';
            lcd_row[1][16] = '\0';
        }
    }

    lcd.update(lcd_row[0], lcd_row[1]);
}

// =============================================================================
// Publish state sensor ke MQTT
// =============================================================================
static void publishSensor(uint32_t now_ms) {
    if (!net || !net->mqttOk()) return;
    const DhtReading& rd = sensor.reading();
    char buf[16];

    // Publish suhu — angka mentah
    if (logic::Payload::sensorValue(rd.valid ? rd.temperature : NAN, buf, sizeof(buf)))
        net->publish(topics->sensorTemp(), buf, false);

    // Publish kelembapan — angka mentah
    if (logic::Payload::sensorValue(rd.valid ? rd.humidity : NAN, buf, sizeof(buf)))
        net->publish(topics->sensorHumidity(), buf, false);

    (void)now_ms; // tidak dipakai setelah refactor
}

// =============================================================================
// Apply lamp state
// =============================================================================
static void applyLampState(bool power, uint8_t brightness, bool publish_now) {
    if (power != led.isOn()) led.setOn(power);
    if (power && brightness != led.brightness()) {
        led.setBrightness(brightness);
        storage.saveBrightness(brightness);
    }

    if (publish_now && net && net->mqttOk()) {
        char buf[48];
        logic::Payload::lampState(led.isOn(), led.brightness(), buf, sizeof(buf));
        net->publish(topics->lampState(), buf, true);
    }
}

// =============================================================================
// Callback perintah MQTT lamp/set
// =============================================================================
static void onLampCmd(const char* payload, size_t len) {
    logic::LampCmd cmd = logic::Payload::parseLampCmd(payload, len);
    if (!cmd.valid) {
        Serial.printf("[MQTT] Payload lamp/set tidak valid: %.*s\n",
                      (int)len, payload);
        return;
    }

    bool    power      = led.isOn();
    uint8_t brightness = led.brightness();

    using F = logic::LampCmd::Field;
    if (cmd.field == F::POWER || cmd.field == F::BOTH) power = cmd.power;
    if (cmd.field == F::BRIGHTNESS || cmd.field == F::BOTH) brightness = cmd.brightness;

    applyLampState(power, brightness, true);
}

// =============================================================================
// Penanganan tombol
// =============================================================================
static void handleButton(BtnPress& press, uint32_t now_ms) {
    switch (press.id) {

    case BtnId::LAMP:
        if (press.type == PressType::SHORT) {
            applyLampState(!led.isOn(), led.brightness(), true);
        } else {
            if (sleep_timer_active) {
                sleep_timer_active = false;
                snprintf(lcd_msg, sizeof(lcd_msg), "Timer dibatalkan");
                lcd_msg_end_ms = now_ms + 1000UL;
            } else if (led.isOn()) {
                sleep_timer_active = true;
                sleep_timer_end_ms = now_ms + ((uint32_t)SLEEP_TIMER_MIN * 60000UL);
                snprintf(lcd_msg, sizeof(lcd_msg), "Timer aktif %dm", SLEEP_TIMER_MIN);
                lcd_msg_end_ms = now_ms + 1000UL;
            }
        }
        break;

    case BtnId::SENSOR:
        if (press.type == PressType::SHORT) {
            lcd_page = (lcd_page + 1) % LCD_PAGE_COUNT;
            last_page_change_ms = now_ms;
        } else {
            publishSensor(now_ms);
            last_sensor_pub_ms = now_ms;
            snprintf(lcd_msg, sizeof(lcd_msg), "Kirim data...   ");
            lcd_msg_end_ms = now_ms + 1000UL;
        }
        break;

    case BtnId::UP:
        if (press.type == PressType::SHORT) {
            if (!led.isOn()) {
                applyLampState(true, storage.loadBrightness(), true);
            } else {
                uint8_t br = led.brightness();
                br = (br + BTN_BRIGHTNESS_STEP > 100) ? 100 : br + BTN_BRIGHTNESS_STEP;
                applyLampState(true, br, true);
            }
        }
        break;

    case BtnId::DOWN:
        if (press.type == PressType::SHORT) {
            if (!led.isOn()) break;
            int br = static_cast<int>(led.brightness()) - BTN_BRIGHTNESS_STEP;
            if (br < LED_MIN_PERCENT) br = LED_MIN_PERCENT;
            applyLampState(true, static_cast<uint8_t>(br), true);
        }
        break;

    default: break;
    }
}

// =============================================================================
// setup()
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("[fw-clock] Booting...");

    storage.begin();
    uint8_t saved_brightness = storage.loadBrightness();

    lcd.begin();
    sensor.begin();

    led.begin();
    led.setOn(true);
    led.setBrightness(saved_brightness);
    led.setOn(false); // mati saat boot, kecerahan tersimpan di last_on_pct_

    buttons.begin();

    buildDeviceId();
    Serial.printf("[fw-clock] Device ID: %s\n", device_id);

    topics    = new logic::Topics(device_id);
    bool isDht22 = (DHT_TYPE == DHT22);
    formatter = new logic::Format(isDht22);

    logic::Payload::discovery(device_id, FW_VERSION, discovery_buf, sizeof(discovery_buf));

    net = new NetService(
        WIFI_SSID, WIFI_PASS,
        MQTT_HOST, MQTT_PORT,
        MQTT_USER, MQTT_PASS,
        device_id,
        topics->status(),
        topics->lampSet(),
        topics->discovery(),
        discovery_buf,
        onLampCmd
    );
    net->begin();

    lcd_page            = 0;
    last_page_change_ms = millis();
    last_sensor_pub_ms  = millis();

    Serial.println("[fw-clock] Setup selesai.");
}

// =============================================================================
// loop()
// =============================================================================
static bool time_started = false;

void loop() {
    uint32_t now = millis();

    net->tick(now);

    if (!time_started && net->wifiOk()) {
        timesvc.begin();
        time_started = true;
    }
    if (time_started) timesvc.tick(now);

    sensor.tick(now);
    led.tick(now);

    BtnPress press;
    if (buttons.poll(press)) {
        handleButton(press, now);
    }

    if (lcd_page != 0 && (now - last_page_change_ms >= LCD_AUTO_BACK_MS)) {
        lcd_page = 0;
    }

    // Render LCD dibatasi setiap 200ms agar bus I2C stabil dan tidak flicker
    static uint32_t last_lcd_render_ms = 0;
    if (now - last_lcd_render_ms >= 200) {
        last_lcd_render_ms = now;
        renderLcd(now);
    }

    if (now - last_sensor_pub_ms >= MQTT_SENSOR_PUB_MS) {
        publishSensor(now);
        last_sensor_pub_ms = now;
    }

    if (sleep_timer_active && now >= sleep_timer_end_ms) {
        sleep_timer_active = false;
        applyLampState(false, led.brightness(), true);
    }
}
