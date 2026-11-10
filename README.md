# SmartRoom

Firmware ESP32 untuk monitor ruangan: suhu/kelembapan (DHT22), cahaya (LDR), gerak (PIR),
layar OLED SSD1306, alarm suhu (LED + buzzer + tombol acknowledge), dan telemetri via MQTT.

## Struktur
```
firmware/smartroom/   sketch Arduino (smartroom.ino + modul sensor, alarm, display, net)
diagram.json          rangkaian simulasi Wokwi
wokwi.toml            konfigurasi ekstensi Wokwi untuk VS Code
wokwi/libraries.txt   daftar library untuk wokwi.com (versi web)
platformio.ini        konfigurasi build PlatformIO
.github/workflows/    CI: compile otomatis dengan arduino-cli
```

## Konfigurasi (wajib sebelum build)
```
cp firmware/smartroom/secrets.h.example firmware/smartroom/secrets.h
```
Isi `WIFI_SSID`, `WIFI_PASS`, `MQTT_HOST`, dan `DEVICE_ID`. File `secrets.h` tidak ikut di-commit.
Nilai bawaan (`Wokwi-GUEST`) hanya berfungsi di simulator Wokwi.

## Build: Arduino IDE
1. Boards Manager: install **esp32 by Espressif Systems** (diuji dengan 3.1.1; 2.x juga didukung kode).
2. Library Manager: DHT sensor library 1.4.7, Adafruit Unified Sensor, Adafruit GFX Library,
   Adafruit SSD1306, PubSubClient 2.8, ArduinoJson 7.x.
3. Buka `firmware/smartroom/smartroom.ino` (nama folder harus `smartroom`).
4. Board: **ESP32 Dev Module** -> Verify -> Upload. Serial Monitor 115200 baud.

Atau lewat CLI:
```
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/smartroom
arduino-cli upload  --fqbn esp32:esp32:esp32 -p COMx firmware/smartroom
```

## Build: PlatformIO
```
pio run
pio run -t upload
pio device monitor
```

## VS Code + Wokwi (simulasi)
1. Install ekstensi **PlatformIO IDE** dan **Wokwi Simulator** (VS Code akan menawarkannya otomatis).
2. Aktifkan lisensi gratis Wokwi: F1 -> *Wokwi: Request a New License*.
3. Buat `firmware/smartroom/secrets.h` dari `secrets.h.example` (biarkan `Wokwi-GUEST` untuk simulasi).
4. Build: ikon centang PlatformIO di bilah bawah (atau `pio run`).
5. F1 -> *Wokwi: Start Simulator*. Ulangi langkah 4 setiap kali kode berubah.

Versi web: buat project di wokwi.com, salin isi `firmware/smartroom/*`, `diagram.json`, dan `wokwi/libraries.txt`.

## Pin
| Fungsi | GPIO |
|---|---|
| DHT22 data | 4 |
| LDR (analog) | 34 |
| PIR | 27 |
| LED hijau / merah | 25 / 26 |
| Buzzer | 33 |
| Tombol acknowledge (ke GND) | 14 |
| OLED SDA / SCL | 21 / 22 (alamat 0x3C) |

## MQTT
Topik berawalan `smartroom/<DEVICE_ID>/`:
- `telemetry` — JSON tiap 5 detik (temp_c, humidity, light_raw, motion, alarm, sensor_ok, time)
- `event` — `alarm_on`, `alarm_off`, `alarm_ack`, `threshold_changed`
- `status` — `online` / `offline` (retained, LWT)
- `command` — kirim `{"threshold": 32.5}` untuk mengubah batas alarm (-40..80)

## Catatan
- Jangan beri nama file yang sama dengan header bawaan platform (mis. `Network.h`); di Windows
  nama file tidak membedakan huruf besar-kecil sehingga bentrok.
- Broker publik (`broker.hivemq.com`) tidak aman: siapa pun yang tahu topik bisa mengirim perintah.
  Pakai broker pribadi dengan autentikasi untuk penggunaan nyata.
- DHT22 4 pin memerlukan pull-up 10k antara data dan 3V3.
- Threshold dari MQTT tidak disimpan setelah restart.
