#pragma once
#include <Arduino.h>

using AlarmEventCallback = void (*)(const char* event, float tempC, float threshold);

void alarmBegin(AlarmEventCallback eventCallback);
void alarmUpdate(float tempC);
void alarmSetThreshold(float value);
void alarmPollButton();

float alarmGetThreshold();
float alarmGetLastTemperature();
bool alarmIsOn();
bool alarmIsSilenced();
