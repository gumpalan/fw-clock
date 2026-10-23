#include <Arduino.h>

// TODO: Sertakan "secrets.h" saat pengembangan lokal
// TODO: Tambahkan driver display 4-digit/LCD, audio MAX98357A, kontrol MOSFET LED, dan client MQTT

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("[fw-clock] Gumpalan ESP32 Clock Initialized.");
}

void loop() {
    // Loop minimal kerangka firmware
    delay(1000);
}
