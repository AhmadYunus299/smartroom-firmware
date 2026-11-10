#pragma once
#include <Arduino.h>
#include "sensor.h"

using ThresholdCommandCallback = void (*)(float value);

void netBegin(ThresholdCommandCallback commandCallback);
void netLoop();
bool netPublishTelemetry(const SensorData& data, bool alarm);
void netPublishEvent(const char* event, float tempC, float threshold);
bool netWifiConnected();
bool netMqttConnected();
