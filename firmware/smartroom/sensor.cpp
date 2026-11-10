#include "sensor.h"
#include "config.h"
#include <DHT.h>

namespace {
  DHT dht(PIN_DHT, DHT22);
}

void sensorBegin() {
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LDR, INPUT);
  dht.begin();
}

void sensorRead(SensorData& data) {
  const float temp = dht.readTemperature();
  const float humidity = dht.readHumidity();

  const bool valid =
      !isnan(temp) && !isnan(humidity) &&
      temp > -40.0f && temp < 80.0f &&
      humidity >= 0.0f && humidity <= 100.0f;

  if (valid) {
    data.tempC = temp;
    data.humidity = humidity;
    data.ok = true;
    data.failCount = 0;
  } else {
    data.ok = false;
    if (data.failCount < 255) data.failCount++;

    Serial.printf("[SENSOR] DHT gagal dibaca (%u kali beruntun)\n",
                  data.failCount);

    if (data.failCount >= SENSOR_FAIL_LIMIT) {
      data.tempC = NAN;
      data.humidity = NAN;
    }
  }

  data.lightRaw = analogRead(PIN_LDR);
  data.motion = digitalRead(PIN_PIR);
}
