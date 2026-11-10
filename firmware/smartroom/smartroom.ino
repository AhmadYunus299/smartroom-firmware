#include <Arduino.h>
#include "config.h"
#include "sensor.h"
#include "alarm.h"
#include "display.h"
#include "net.h"

namespace {
  SensorData sensorData;
  unsigned long lastRead = 0;
  unsigned long lastRender = 0;
}

void setup() {
  Serial.begin(115200);

  sensorBegin();
  displayBegin();
  displayBoot("Menghubungkan...");

  // netBegin menerima callback untuk perintah threshold dari MQTT.
  netBegin(alarmSetThreshold);

  // alarmBegin menerima callback untuk mengirim event ke MQTT.
  alarmBegin(netPublishEvent);
}

void loop() {
  const unsigned long now = millis();

  netLoop();
  alarmPollButton();

  if (now - lastRead >= READ_INTERVAL_MS) {
    lastRead = now;

    sensorRead(sensorData);
    alarmUpdate(sensorData.tempC);

    const bool published = netPublishTelemetry(sensorData, alarmIsOn());
    Serial.printf("[MQTT] Telemetry %s\n", published ? "terkirim" : "gagal/belum terhubung");

    lastRender = 0; // OLED memperbarui data terbaru.
  }

  if (now - lastRender >= RENDER_INTERVAL_MS) {
    lastRender = now;

    displayRender(sensorData,
                  alarmGetThreshold(),
                  alarmIsOn(),
                  alarmIsOn() && alarmIsSilenced(),
                  netWifiConnected(),
                  netMqttConnected());
  }
}
