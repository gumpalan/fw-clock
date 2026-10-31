#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <cstdint>
#include "../../include/config.h"

// =============================================================================
// NetService.h — WiFi + MQTT (PubSubClient) dengan reconnect non-blocking
// =============================================================================

// Callback saat perintah lamp/set diterima.
// payload: buffer raw, len: panjang payload.
using MqttCmdCallback = void(*)(const char* payload, size_t len);

class NetService {
public:
    NetService(const char* wifi_ssid, const char* wifi_pass,
               const char* mqtt_host, uint16_t mqtt_port,
               const char* mqtt_user, const char* mqtt_pass,
               const char* device_id,
               const char* topic_status,
               const char* topic_lamp_set,
               const char* topic_discovery,
               const char* discovery_payload,
               MqttCmdCallback on_lamp_cmd);

    void begin();
    void tick(uint32_t now_ms);

    // Publish ke topik. Return false jika belum terhubung.
    bool publish(const char* topic, const char* payload,
                 bool retained = false);

    bool wifiOk() const;
    bool mqttOk() { return mqtt_.connected(); }

    // Isi IP string ke buf[16].
    void ipStr(char* buf) const;

private:
    const char*    wifi_ssid_;
    const char*    wifi_pass_;
    const char*    mqtt_host_;
    uint16_t       mqtt_port_;
    const char*    mqtt_user_;
    const char*    mqtt_pass_;
    const char*    device_id_;
    const char*    topic_status_;
    const char*    topic_lamp_set_;
    const char*    topic_discovery_;
    const char*    discovery_payload_;
    MqttCmdCallback on_lamp_cmd_;

    WiFiClient     wifi_client_;
    PubSubClient   mqtt_;

    // Backoff reconnect
    uint32_t       next_connect_ms_;
    uint32_t       backoff_s_;   // 1, 2, 4, 8, 16, 30

    bool           wifi_was_ok_;

    void connectWifi();
    void connectMqtt();
    void onConnect();

    // Callback PubSubClient (static, dispatch ke instance via closure).
    static NetService* instance_;
    static void mqttCallback(const char* topic, uint8_t* payload, unsigned int len);
};
