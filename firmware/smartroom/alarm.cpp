#include "alarm.h"
#include "config.h"

namespace {
  float threshold = DEFAULT_THRESHOLD;
  float lastTemp = NAN;
  bool alarmOn = false;
  bool silenced = false;

  int btnStable = HIGH;
  int btnLastRead = HIGH;
  unsigned long btnLastChange = 0;

  AlarmEventCallback eventCallback = nullptr;

  void emitEvent(const char* event, float tempC) {
    if (eventCallback) eventCallback(event, tempC, threshold);
  }

  void buzzerInit() {
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PIN_BUZZER, BUZZER_FREQ, 8);
  #else
    ledcSetup(BUZZER_CH, BUZZER_FREQ, 8);
    ledcAttachPin(PIN_BUZZER, BUZZER_CH);
  #endif
  }

  void buzzerSet(bool on) {
    const uint32_t duty = on ? 128 : 0;
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PIN_BUZZER, duty);
  #else
    ledcWrite(BUZZER_CH, duty);
  #endif
  }

  void applyOutputs() {
    digitalWrite(PIN_LED_R, alarmOn ? HIGH : LOW);
    digitalWrite(PIN_LED_G, alarmOn ? LOW : HIGH);
    buzzerSet(alarmOn && !silenced);
  }
}

void alarmBegin(AlarmEventCallback callback) {
  eventCallback = callback;

  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  buzzerInit();
  applyOutputs();
}

void alarmUpdate(float tempC) {
  if (isnan(tempC)) return;

  lastTemp = tempC;

  if (!alarmOn && tempC > threshold) {
    alarmOn = true;
    silenced = false;
    Serial.println("[ALARM] Aktif");
    emitEvent("alarm_on", tempC);
  } else if (alarmOn && tempC < threshold - ALARM_HYSTERESIS) {
    alarmOn = false;
    silenced = false;
    Serial.println("[ALARM] Normal");
    emitEvent("alarm_off", tempC);
  }

  applyOutputs();
}

void alarmSetThreshold(float value) {
  threshold = value;
  emitEvent("threshold_changed", lastTemp);

  if (!isnan(lastTemp)) {
    const bool shouldAlarm = lastTemp > threshold;

    if (!shouldAlarm) silenced = false;

    if (shouldAlarm != alarmOn) {
      emitEvent(shouldAlarm ? "alarm_on" : "alarm_off", lastTemp);
    }

    alarmOn = shouldAlarm;
  }

  applyOutputs();
}

void alarmPollButton() {
  const int reading = digitalRead(PIN_BTN);

  if (reading != btnLastRead) {
    btnLastChange = millis();
    btnLastRead = reading;
  }

  if (millis() - btnLastChange > DEBOUNCE_MS && reading != btnStable) {
    btnStable = reading;

    if (btnStable == LOW && alarmOn && !silenced) {
      silenced = true;
      applyOutputs();
      Serial.println("[ALARM] Di-acknowledge");
      emitEvent("alarm_ack", lastTemp);
    }
  }
}

float alarmGetThreshold() { return threshold; }
float alarmGetLastTemperature() { return lastTemp; }
bool alarmIsOn() { return alarmOn; }
bool alarmIsSilenced() { return silenced; }
