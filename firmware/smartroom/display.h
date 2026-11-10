#pragma once
#include <Arduino.h>
#include "sensor.h"

bool displayBegin();
void displayBoot(const char* message);
void displayRender(const SensorData& data, float threshold, bool alarm,
                   bool acknowledged, bool wifiConnected, bool mqttConnected);
