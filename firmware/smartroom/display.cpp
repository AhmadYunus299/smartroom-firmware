#include "display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
  Adafruit_SSD1306 oled(OLED_W, OLED_H, &Wire, -1);
  bool oledOk = false;
}

bool displayBegin() {
  Wire.begin(); // SDA 21, SCL 22
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  if (!oledOk) {
    Serial.println("[OLED] Tidak ditemukan, lanjut tanpa layar");
    return false;
  }

  Wire.setClock(400000);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  return true;
}

void displayBoot(const char* message) {
  if (!oledOk) return;

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Smart Room Monitor");
  oled.println(message);
  oled.display();
}

void displayRender(const SensorData& data, float threshold, bool alarm,
                   bool acknowledged, bool wifiConnected, bool mqttConnected) {
  if (!oledOk) return;

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  oled.setCursor(0, 0);
  oled.printf("WiFi:%s MQTT:%s",
              wifiConnected ? "OK" : "--",
              mqttConnected ? "OK" : "--");
  oled.drawLine(0, 10, OLED_W, 10, SSD1306_WHITE);

  oled.setCursor(0, 14);
  if (isnan(data.tempC)) oled.print("Suhu  : --");
  else oled.printf("Suhu  : %.1f C", data.tempC);

  oled.setCursor(0, 24);
  if (isnan(data.humidity)) oled.print("Lembab: --");
  else oled.printf("Lembab: %.1f %%", data.humidity);

  oled.setCursor(0, 34);
  oled.printf("Cahaya: %d", data.lightRaw);

  oled.setCursor(0, 44);
  oled.printf("Gerak : %s Max:%.1f",
              data.motion ? "YA" : "tdk", threshold);

  oled.setCursor(0, 55);
  if (data.failCount >= SENSOR_FAIL_LIMIT) oled.print("!! SENSOR ERROR !!");
  else if (alarm) oled.print(acknowledged ? "ALARM (ack)" : "!! ALARM SUHU !!");
  else oled.print("Normal");

  oled.display();
}
