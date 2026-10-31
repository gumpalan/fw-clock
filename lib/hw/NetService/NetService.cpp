#include "NetService.h"
#include <Arduino.h>

// Singleton pointer untuk callback PubSubClient (static)
NetService* NetService::instance_ = nullptr;

NetService::NetService(const char* wifi_ssid, const char* wifi_pass,
                       const char* mqtt_host, uint16_t mqtt_port,
                       const char* mqtt_user, const char* mqtt_pass,
                       const char* device_id,
                       const char* topic_status,
                       const char* topic_lamp_set,
                       const char* topic_discovery,
                       const char* discovery_payload,
                       MqttCmdCallback on_lamp_cmd)
    : wifi_ssid_(wifi_ssid), wifi_pass_(wifi_pass),
      mqtt_host_(mqtt_host), mqtt_port_(mqtt_port),
      mqtt_user_(mqtt_user), mqtt_pass_(mqtt_pass),
      device_id_(device_id),
      topic_status_(topic_status),
      topic_lamp_set_(topic_lamp_set),
      topic_discovery_(topic_discovery),
      discovery_payload_(discovery_payload),
      on_lamp_cmd_(on_lamp_cmd),
      mqtt_(wifi_client_),
      next_connect_ms_(0), backoff_s_(1),
      wifi_was_ok_(false)
{
    instance_ = this;
}

void NetService::begin() {
    mqtt_.setServer(mqtt_host_, mqtt_port_);
    mqtt_.setBufferSize(MQTT_BUFFER_SIZE);
    mqtt_.setCallback(mqttCallback);
    connectWifi();
}

void NetService::tick(uint32_t now_ms) {
    // Reconnect WiFi jika terputus
    if (WiFi.status() != WL_CONNECTED) {
        if (wifi_was_ok_) {
            Serial.println("[WiFi] Terputus, mencoba reconnect...");
            wifi_was_ok_ = false;
        }
        if (now_ms >= next_connect_ms_) {
            connectWifi();
            // Backoff: tunggu backoff_s_ detik sebelum coba lagi
            next_connect_ms_ = now_ms + (backoff_s_ * 1000UL);
            backoff_s_ = (backoff_s_ * 2 > MQTT_BACKOFF_MAX_S)
                         ? MQTT_BACKOFF_MAX_S : backoff_s_ * 2;
        }
        return;
    }

    // WiFi OK
    if (!wifi_was_ok_) {
        wifi_was_ok_ = true;
        backoff_s_   = 1;
        Serial.printf("[WiFi] Terhubung. IP: %s\n",
                      WiFi.localIP().toString().c_str());
    }

    // MQTT loop
    if (mqtt_.connected()) {
        mqtt_.loop();
    } else {
        if (now_ms >= next_connect_ms_) {
            connectMqtt();
            next_connect_ms_ = now_ms + (backoff_s_ * 1000UL);
            backoff_s_ = (backoff_s_ * 2 > MQTT_BACKOFF_MAX_S)
                         ? MQTT_BACKOFF_MAX_S : backoff_s_ * 2;
        }
    }
}

void NetService::connectWifi() {
    Serial.printf("[WiFi] Menghubungkan ke %s...\n", wifi_ssid_);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid_, wifi_pass_);
    // Tidak blocking — cek status di tick()
}

void NetService::connectMqtt() {
    if (WiFi.status() != WL_CONNECTED) return;

    // Bangun client id unik
    char client_id[40];
    snprintf(client_id, sizeof(client_id), "gumpalan-%s", device_id_);

    Serial.printf("[MQTT] Menghubungkan ke %s:%d sebagai %s...\n",
                  mqtt_host_, mqtt_port_, client_id);

    // Last Will: topic_status "offline" retained
    bool ok = mqtt_.connect(client_id,
                            mqtt_user_, mqtt_pass_,
                            topic_status_, 0, true, "offline");
    if (ok) {
        backoff_s_ = 1;
        onConnect();
    } else {
        Serial.printf("[MQTT] Gagal (state=%d)\n", mqtt_.state());
    }
}

void NetService::onConnect() {
    Serial.println("[MQTT] Terhubung.");

    // Publish status "online" retained
    mqtt_.publish(topic_status_, "online", true);

    // Subscribe lamp/set
    mqtt_.subscribe(topic_lamp_set_);
    Serial.printf("[MQTT] Subscribe: %s\n", topic_lamp_set_);

    // Publish discovery retained
    mqtt_.publish(topic_discovery_, discovery_payload_, true);
    Serial.printf("[MQTT] Discovery publish ke %s\n", topic_discovery_);
}

bool NetService::publish(const char* topic, const char* payload, bool retained) {
    if (!mqtt_.connected()) return false;
    return mqtt_.publish(topic, payload, retained);
}

bool NetService::wifiOk() const {
    return WiFi.status() == WL_CONNECTED;
}

void NetService::ipStr(char* buf) const {
    if (WiFi.status() == WL_CONNECTED) {
        strncpy(buf, WiFi.localIP().toString().c_str(), 15);
        buf[15] = '\0';
    } else {
        strncpy(buf, "---", 15);
    }
}

// static callback PubSubClient
void NetService::mqttCallback(const char* topic, uint8_t* payload, unsigned int len) {
    if (!instance_) return;

    if (strcmp(topic, instance_->topic_lamp_set_) == 0) {
        if (instance_->on_lamp_cmd_) {
            instance_->on_lamp_cmd_(reinterpret_cast<const char*>(payload), len);
        }
    }
}
