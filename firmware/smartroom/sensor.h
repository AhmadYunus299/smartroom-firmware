#pragma once
#include <Arduino.h>

struct SensorData {
  float tempC = NAN;
  float humidity = NAN;
  int lightRaw = 0;
  bool motion = false;
  bool ok = false;
  uint8_t failCount = 0;
};

void sensorBegin();
void sensorRead(SensorData& data);
