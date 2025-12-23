# SmartLove

ESP32-basiertes SmartLove Projekt - Kompatibel mit ESP-IDF 4.x und 5.x

## 🚀 Features

- ✅ ESP-IDF 4.x und 5.x kompatibel
- ✅ Modulare Komponenten-Architektur
- ✅ **Zentrale Konfiguration** (`smartlove_config.h`)
- ✅ Umfassende System-Informationen
- ✅ WiFi Manager mit Captive Portal
- ✅ MQTT Client (Public HiveMQ Broker)
- ✅ WS2812B LED-Steuerung via MQTT
- ✅ Button-Handler mit Tastendruck-Dauer
- ✅ JSON-basierte Befehle
- ✅ Vorbereitet für Bluetooth, Sensoren

## ⚙️ Zentrale Konfiguration

Alle GPIO-Pins, MQTT-Einstellungen, WiFi-Parameter und weitere Konfigurationen befinden sich in einer zentralen Datei:

**📁 Datei**: `components/smartlove_config/include/smartlove_config.h`

### Wichtige Einstellungen

| Einstellung | Wert | Beschreibung |
|-------------|------|--------------|
| `SMARTLOVE_USE_WIFI_CONFIG_PORTAL` | 1 | WiFi-Portal aktivieren (0 = nur Default-Credentials) |
| `SMARTLOVE_DEFAULT_WIFI_SSID` | "" | Standard WiFi SSID (leer = Portal nutzen) |
| `SMARTLOVE_DEFAULT_WIFI_PASSWORD` | "" | Standard WiFi Passwort |
| `SMARTLOVE_GPIO_LED_STRIP` | 27 | GPIO für WS2812B LEDs |
| `SMARTLOVE_GPIO_BUTTON` | 17 | GPIO für Taster |
| `SMARTLOVE_LED_COUNT` | 2 | Anzahl der LEDs |
| `SMARTLOVE_MQTT_BROKER_URI` | mqtt://broker.hivemq.com | MQTT Broker URL |

### WiFi-Modus Konfiguration

**Mit Captive Portal (Standard):**
```c
#define SMARTLOVE_USE_WIFI_CONFIG_PORTAL    1
```
→ Wenn keine gespeicherten Credentials vorhanden, startet AP mit Portal

**Ohne Portal (nur Default-Credentials):**
```c
#define SMARTLOVE_USE_WIFI_CONFIG_PORTAL    0
#define SMARTLOVE_DEFAULT_WIFI_SSID         "MeinWiFi"
#define SMARTLOVE_DEFAULT_WIFI_PASSWORD     "MeinPasswort"
```
→ Verbindet direkt mit den angegebenen Credentials

## 🎨 LED Controller

### Hardware
- **GPIO Pin**: 27 (konfigurierbar in `smartlove_config.h`)
- **LED Typ**: WS2812B (NeoPixel)
- **Anzahl LEDs**: 2 (konfigurierbar)

### MQTT Befehle

**Topic**: `SmartLove/<CHIP_ID>/in`

#### JSON LED-Steuerung
Alle Parameter sind optional und können kombiniert werden:

```json
{"intensity": 255, "color": {"r": 255, "g": 0, "b": 0}}
```
- `intensity`: 0-255 (0 = LEDs AUS, >0 = LEDs AN)
- `color`: RGB-Werte (0-255 pro Kanal)
- `show`: Animation ("BLINK", "NONE", "FADE")

**Beispiele:**
```json
// Rot mit voller Helligkeit
{"intensity": 255, "color": {"r": 255, "g": 0, "b": 0}}

// Blau mit halber Helligkeit
{"intensity": 128, "color": {"r": 0, "g": 0, "b": 255}}

// Grün blinkend
{"intensity": 200, "color": {"r": 0, "g": 255, "b": 0}, "show": "BLINK"}

// LEDs ausschalten
{"intensity": 0}

// Nur Farbe ändern (Helligkeit bleibt)
{"color": {"r": 255, "g": 255, "b": 0}}
// Sanftes Überblenden zu Blau in 2 Sekunden
{"show": "FADE", "color": {"r": 0, "g": 0, "b": 255}, "fade_ms": 2000}
```


#### FADE-Animation

Mit `"show": "FADE"` wird die LED von der aktuellen Farbe zur Ziel-Farbe übergeblendet. Die Startfarbe ist immer der aktuelle LED-Zustand zum Zeitpunkt des Befehls.

- `color`: Ziel-Farbe (Pflichtfeld)
- `fade_ms`: Dauer in Millisekunden (Pflichtfeld)

Nach Ablauf der Zeit bleibt die LED auf der Ziel-Farbe stehen.

**Beispiel:**
```json
{"show": "FADE", "color": {"r": 255, "g": 0, "b": 255}, "fade_ms": 1500}
```

#### Text-Befehle
```
LED_ON      - LEDs einschalten
LED_OFF     - LEDs ausschalten
STATUS      - System-Status mit LED-Zustand
PING        - Verbindungstest (Antwort: PONG)
```

## 🔘 Button Handler

### Hardware
- **GPIO Pin**: 17 (konfigurierbar in `smartlove_config.h`)
- **Logik**: Active LOW (interner Pull-Up)
- **Debounce**: 50ms

### MQTT Events

Bei jedem Tastendruck wird ein JSON-Event gesendet:

**Topic**: `SmartLove/<CHIP_ID>/out`

```json
{
  "event": "button_press",
  "duration_ms": 1500,
  "uptime": 123456
}
```

- `duration_ms`: Tastendruck-Dauer in Millisekunden
- `uptime`: System-Uptime in Millisekunden

## 📡 MQTT Verbindung

- **Broker**: mqtt://broker.hivemq.com:1883 (öffentlich, kein TLS)
- **Client ID**: `smartlove_<CHIP_ID>`
- **Subscribe Topic**: `SmartLove/<CHIP_ID>/in`
- **Publish Topic**: `SmartLove/<CHIP_ID>/out`
- **Heartbeat**: Alle 60 Sekunden

### Startup Message
Bei Verbindung sendet der ESP32 automatisch System-Informationen:
```json
{
  "event": "startup",
  "chip_id": "30AEA4224918",
  "idf_version": "v4.4.7",
  "free_heap": 218684,
  "model": "ESP32-D0WDQ6"
}
```

## 📋 Voraussetzungen

### Aktuell (IDF 4.x)
```bash
# ESP-IDF 4.4.7
. ~/esp/v4.4.7/esp-idf/export.sh
```

### Migration zu IDF 5.x
```bash
# ESP-IDF 5.3 oder höher
. ~/esp/v5.3/esp-idf/export.sh
```

## 🔧 Installation

1. **Repository klonen** (falls noch nicht geschehen)
```bash
git clone <repository-url>
cd smartlove
```

2. **ESP-IDF Umgebung aktivieren**
```bash
# Für IDF 4.x
. ~/esp/v4.4.7/esp-idf/export.sh

# ODER für IDF 5.x (nach Migration)
. ~/esp/v5.3/esp-idf/export.sh
```

3. **Target setzen**
```bash
idf.py set-target esp32
```

4. **Konfigurieren** (optional)
```bash
idf.py menuconfig
```

## 🏗️ Build

### Methode 1: Mit Build-Skript (empfohlen)
```bash
# Bauen
./build.sh build

# Flash und Monitor
./build.sh flash-monitor /dev/cu.usbserial-0001

# Weitere Optionen anzeigen
./build.sh help
```

### Methode 2: Mit idf.py direkt
```bash
# Bauen
idf.py build

# Flashen
idf.py -p /dev/cu.usbserial-0001 flash

# Monitor
idf.py -p /dev/cu.usbserial-0001 monitor

# Alles zusammen
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

## 📊 Projekt-Struktur

```
smartlove/
├── CMakeLists.txt              # Haupt-Build-Konfiguration
├── sdkconfig                   # SDK-Konfiguration
├── build.sh                    # Build-Skript
├── MIGRATION_NOTES.md          # Migrations-Dokumentation
├── README.md                   # Diese Datei
├── main/
│   ├── CMakeLists.txt
│   └── main.c                  # Haupt-Anwendung
└── components/
    ├── README.md
    └── smartlove_utils/        # Utility-Komponente
        ├── CMakeLists.txt
        ├── include/
        │   └── smartlove_utils.h
        └── smartlove_utils.c
```

## 🔄 Migration zu ESP-IDF 5.x

Siehe [MIGRATION_NOTES.md](MIGRATION_NOTES.md) für detaillierte Migrations-Anweisungen.

### Quick-Start Migration:

1. **ESP-IDF 5.3 installieren**
```bash
cd ~/esp
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git v5.3/esp-idf
cd v5.3/esp-idf
./install.sh
```

2. **Projekt für IDF 5.x vorbereiten**
```bash
cd /path/to/smartlove
. ~/esp/v5.3/esp-idf/export.sh
./build.sh reconfigure
```

3. **Testen**
```bash
./build.sh build
./build.sh flash-monitor /dev/cu.usbserial-0001
```

## 🧪 Testen

Nach dem Flashen sollten Sie folgende Ausgabe sehen:

```
=================================
   SmartLove System Info
=================================
Chip: esp32
Cores: 2
Features: WiFi/BT/BLE
Silicon revision: X
Flash size: 4 MB
Free heap: XXXXX bytes
IDF Version: vX.X.X
=================================
Application started successfully!
IDF 5.x compatible: Yes/No
Ready for development...
Heartbeat: 0 | Uptime: 0 ms | Free heap: XXXXX bytes
```

## 📝 Nächste Schritte

### Geplante Features:
- [ ] WiFi Manager Komponente
- [ ] Bluetooth Low Energy (BLE) Kommunikation
- [ ] Sensor-Integration
- [ ] Web-Server für Konfiguration
- [ ] OTA (Over-The-Air) Updates
- [ ] Daten-Logging
- [ ] Cloud-Anbindung

### Entwicklung:

1. **Neue Komponente erstellen**
```bash
mkdir -p components/my_component/include
touch components/my_component/CMakeLists.txt
touch components/my_component/include/my_component.h
touch components/my_component/my_component.c
```

2. **CMakeLists.txt für neue Komponente**
```cmake
idf_component_register(
    SRCS "my_component.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer
)
```

3. **In main/CMakeLists.txt einbinden**
```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES smartlove_utils my_component
)
```

## 🐛 Debugging

### Serial Monitor mit Logs
```bash
idf.py -p /dev/cu.usbserial-0001 monitor
```

Zum Beenden: `Ctrl+]`

### Log-Level anpassen
In `sdkconfig` oder via menuconfig:
```
Component config → Log output → Default log verbosity → Debug
```

### Core Dump bei Crashes
```bash
idf.py -p /dev/cu.usbserial-0001 monitor
# Bei Crash wird automatisch ein Core Dump ausgegeben
```

## 📚 Ressourcen

- [ESP-IDF Dokumentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32 Datenblatt](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [Migration Guide zu IDF 5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/index.html)

## 🤝 Beitragen

1. Fork erstellen
2. Feature Branch erstellen (`git checkout -b feature/AmazingFeature`)
3. Änderungen committen (`git commit -m 'Add some AmazingFeature'`)
4. Branch pushen (`git push origin feature/AmazingFeature`)
5. Pull Request erstellen

## 📄 Lizenz

[Lizenz hier einfügen]

## ✨ Status

- ✅ Grundprojekt eingerichtet
- ✅ IDF 4.x kompatibel
- ✅ IDF 5.x vorbereitet
- ✅ Komponenten-Struktur erstellt
- ⏳ Migration zu IDF 5.x ausstehend
- ⏳ Feature-Entwicklung

## 📧 Kontakt

Marcel Seerig - [GitHub](https://github.com/mseerig)

Projekt Link: [https://github.com/mseerig/SmartLove](https://github.com/mseerig/SmartLove)
