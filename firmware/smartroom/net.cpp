#include "net.h"
#include "config.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

namespace {
  WiFiClient wifiClient;
  PubSubClient mqtt(wifiClient);

  String deviceId;
  String topicTelemetry;
  String topicStatus;
  String topicCommand;
  String topicEvent;

  unsigned long lastWifiAttempt = 0;
  unsigned long lastMqttAttempt = 0;
  bool mqttTried = false;

  ThresholdCommandCallback thresholdCallback = nullptr;

  String sanitizeId(const char* raw) {
    String result(raw);
    for (size_t i = 0; i < result.length(); i++) {
      const char c = result[i];
      if (c == '#' || c == '+' || c == '/' || c == ' ') {
        result.setCharAt(i, '-');
      }
    }
    return result;
  }

  String nowString() {
    const time_t now = time(nullptr);
    if (now < 1700000000) return String("");

    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    char buffer[24];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    return String(buffer);
  }

  void onMessage(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<128> doc;
    const DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
      Serial.printf("[MQTT] JSON error: %s\n", error.c_str());
      return;
    }

    if (!(doc["threshold"].is<float>() || doc["threshold"].is<int>())) {
      Serial.println("[MQTT] Perintah tanpa field 'threshold', diabaikan");
      return;
    }

    const float value = doc["threshold"].as<float>();
    if (value <= -40.0f || value >= 80.0f) {
      Serial.println("[MQTT] Threshold di luar rentang, diabaikan");
      return;
    }

    Serial.printf("[MQTT] threshold -> %.1f\n", value);
    if (thresholdCallback) thresholdCallback(value);
  }

  void mqttConnect() {
    const String clientId =
        deviceId + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.printf("[MQTT] Connecting as %s...\n", clientId.c_str());

    bool connected = false;

  #ifdef MQTT_USER
    connected = mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS_MQTT,
                             topicStatus.c_str(), 1, true, "offline");
  #else
    connected = mqtt.connect(clientId.c_str(), nullptr, nullptr,
                             topicStatus.c_str(), 1, true, "offline");
  #endif

    if (connected) {
      Serial.println("[MQTT] Connected");
      mqtt.publish(topicStatus.c_str(), "online", true);
      mqtt.subscribe(topicCommand.c_str(), 1);
    } else {
      Serial.printf("[MQTT] Gagal, rc=%d\n", mqtt.state());
    }
  }
}

void netBegin(ThresholdCommandCallback commandCallback) {
  thresholdCallback = commandCallback;

  deviceId = sanitizeId(DEVICE_ID);
  topicTelemetry = "smartroom/" + deviceId + "/telemetry";
  topicStatus    = "smartroom/" + deviceId + "/status";
  topicCommand   = "smartroom/" + deviceId + "/command";
  topicEvent     = "smartroom/" + deviceId + "/event";

  Serial.printf("[MQTT] Topik telemetri: %s\n", topicTelemetry.c_str());

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiAttempt = millis();

  Serial.printf("[WiFi] Menghubungkan ke %s...\n", WIFI_SSID);

  configTime(GMT_OFFSET_S, 0, "pool.ntp.org", "time.google.com");

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(3);
}

void netLoop() {
  const unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    mqttTried = false;

    if (now - lastWifiAttempt >= WIFI_RETRY_MS) {
      lastWifiAttempt = now;
      Serial.println("[WiFi] Belum terhubung, mencoba lagi...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }

  if (!mqtt.connected()) {
    if (!mqttTried || now - lastMqttAttempt >= MQTT_RETRY_MS) {
      mqttTried = true;
      lastMqttAttempt = now;
      mqttConnect();
    }
    return;
  }

  mqtt.loop();
}

void netPublishEvent(const char* event, float tempC, float threshold) {
  StaticJsonDocument<256> doc;
  doc["event"] = event;

  const String timestamp = nowString();
  if (timestamp.length()) doc["time"] = timestamp;
  else doc["time"] = nullptr;

  doc["uptime_s"] = millis() / 1000;

  if (isnan(tempC)) doc["temp_c"] = nullptr;
  else doc["temp_c"] = round(tempC * 10) / 10.0;

  doc["threshold"] = round(threshold * 10) / 10.0;

  char buffer[256];
  const size_t length = serializeJson(doc, buffer, sizeof(buffer));

  Serial.printf("[EVENT] %s\n", buffer);

  if (mqtt.connected()) {
    mqtt.publish(topicEvent.c_str(),
                 reinterpret_cast<const uint8_t*>(buffer), length, false);
  }
}

bool netPublishTelemetry(const SensorData& data, bool alarm) {
  StaticJsonDocument<384> doc;

  doc["device_id"] = deviceId;
  if (isnan(data.tempC)) doc["temp_c"] = nullptr;
  else doc["temp_c"] = round(data.tempC * 10) / 10.0;

  if (isnan(data.humidity)) doc["humidity"] = nullptr;
  else doc["humidity"] = round(data.humidity * 10) / 10.0;

  doc["light_raw"] = data.lightRaw;
  doc["motion"] = data.motion;
  doc["alarm"] = alarm;
  doc["uptime_s"] = millis() / 1000;
  doc["sensor_ok"] = data.ok;

  const String timestamp = nowString();
  if (timestamp.length()) doc["time"] = timestamp;
  else doc["time"] = nullptr;

  char buffer[384];
  const size_t length = serializeJson(doc, buffer, sizeof(buffer));

  Serial.printf("[MQTT] Payload: %s\n", buffer);

  if (!mqtt.connected()) return false;

  return mqtt.publish(topicTelemetry.c_str(),
                      reinterpret_cast<const uint8_t*>(buffer), length, false);
}

bool netWifiConnected() { return WiFi.status() == WL_CONNECTED; }
bool netMqttConnected() { return mqtt.connected(); }
