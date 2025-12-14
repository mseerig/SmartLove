# WiFi Manager mit Captive Portal

## 🎯 Features

- ✅ **Auto-Connect**: Automatische Verbindung zu gespeichertem WiFi
- ✅ **Captive Portal**: Browser öffnet sich automatisch zur Konfiguration
- ✅ **Web-Interface**: Modernes, responsives Design
- ✅ **WiFi-Scan**: Verfügbare Netzwerke anzeigen
- ✅ **NVS-Speicherung**: Persistent gespeicherte Credentials
- ✅ **Fallback-Modus**: Automatischer AP-Modus bei Verbindungsfehler
- ✅ **Event-Callbacks**: Ereignis-Benachrichtigungen
- ✅ **IDF 4.x & 5.x**: Vollständig kompatibel

## 📋 Wie es funktioniert

### 1. Erststart (keine gespeicherten Credentials)
```
ESP32 startet → Keine WiFi-Daten → Startet AP-Modus
                                    ↓
                         AP: "SmartLove-XXXX"
                         IP: 192.168.4.1
                                    ↓
                    Captive Portal aktiv
                    DNS Server leitet alles um
```

### 2. Benutzer-Konfiguration
```
Smartphone → Verbindet mit "SmartLove-XXXX"
           → Browser öffnet automatisch
           → Zeigt verfügbare WiFi-Netzwerke
           → Benutzer wählt Netzwerk & gibt Passwort ein
           → Credentials werden gespeichert
           → ESP32 verbindet sich
```

### 3. Normalbetrieb
```
ESP32 startet → Liest gespeicherte Credentials
              → Verbindet automatisch
              → Bei Fehler: Zurück zu AP-Modus
```

## 🚀 Verwendung

### Basis-Setup

```c
#include "wifi_manager.h"

void app_main(void)
{
    // 1. WiFi Manager initialisieren
    wifi_manager_config_t config = wifi_manager_get_default_config();
    
    // Optional: Anpassen
    snprintf(config.ap_ssid, sizeof(config.ap_ssid), "MyDevice-%04X", 
             esp_random() & 0xFFFF);
    config.max_retry_attempts = 10;
    
    ESP_ERROR_CHECK(wifi_manager_init(&config));
    
    // 2. Event-Callback registrieren (optional)
    wifi_manager_register_event_callback(my_callback, NULL);
    
    // 3. Starten
    ESP_ERROR_CHECK(wifi_manager_start());
}
```

### Event-Callback Beispiel

```c
void my_callback(wifi_manager_event_t event, void *user_data)
{
    switch (event) {
        case WIFI_MANAGER_EVENT_STA_CONNECTED:
            printf("WiFi verbunden!\n");
            // Starte MQTT, HTTP-Anfragen, etc.
            break;
            
        case WIFI_MANAGER_EVENT_STA_DISCONNECTED:
            printf("WiFi getrennt\n");
            // Stoppe Netzwerk-Services
            break;
            
        case WIFI_MANAGER_EVENT_AP_STARTED:
            printf("Config-Portal gestartet\n");
            break;
            
        default:
            break;
    }
}
```

### Manuelle Steuerung

```c
// WiFi-Scan starten
wifi_manager_scan();

// Manuell verbinden
wifi_manager_connect("MeinWiFi", "password123", true);

// Credentials löschen (für Neukonfiguration)
wifi_manager_clear_credentials();

// Status abfragen
wifi_manager_status_t status = wifi_manager_get_status();
if (status == WIFI_MANAGER_CONNECTED) {
    // WiFi verbunden
}

// Gespeicherte SSID abrufen
char ssid[33];
wifi_manager_get_saved_ssid(ssid, sizeof(ssid));
```

## 🎨 Web-Interface

Das Captive Portal bietet:

- **Automatisches Öffnen**: Smartphone erkennt Portal automatisch
- **Network-Scan**: Liste aller verfügbaren WiFi-Netzwerke
- **Signal-Stärke**: Visuelle Anzeige der Signalqualität
- **Sicherheits-Icon**: Zeigt ob Netzwerk verschlüsselt ist
- **Responsive Design**: Funktioniert auf allen Geräten
- **Status-Feedback**: Zeigt Verbindungserfolg/-fehler

### URLs

- **Hauptseite**: `http://192.168.4.1/`
- **Scan**: `http://192.168.4.1/scan`
- **Connect**: `http://192.168.4.1/connect` (POST)

## 🔧 Konfiguration

```c
typedef struct {
    char ap_ssid[32];           // AP-Name im Config-Modus
    char ap_password[64];       // AP-Passwort (leer = offen)
    uint8_t ap_channel;         // WiFi-Kanal (1-13)
    uint8_t ap_max_connections; // Max. gleichzeitige Verbindungen
    uint16_t portal_timeout_s;  // Portal-Timeout (0 = kein Timeout)
    uint8_t max_retry_attempts; // Max. Verbindungsversuche
} wifi_manager_config_t;
```

### Standard-Werte

```c
wifi_manager_config_t default_config = {
    .ap_ssid = "SmartLove-Setup",
    .ap_password = "",          // Offenes Netzwerk
    .ap_channel = 1,
    .ap_max_connections = 4,
    .portal_timeout_s = 0,      // Kein Timeout
    .max_retry_attempts = 5
};
```

## 📱 Captive Portal Detection

Der WiFi Manager unterstützt Captive Portal Detection für:

- ✅ **iOS**: Automatisches Popup
- ✅ **Android**: Notification mit "Sign in"
- ✅ **Windows**: Automatisches Öffnen
- ✅ **macOS**: Automatisches Popup
- ✅ **Linux**: Browser-Umleitung

## 🔐 Sicherheit

- **NVS-Verschlüsselung**: Credentials werden im NVS gespeichert
- **WPA2-PSK**: Unterstützung für verschlüsselte Netzwerke
- **Offenes AP**: Optional (Standard: offen für einfache Konfiguration)
- **Timeout**: Optional automatisches Schließen des Portals

## 🐛 Debugging

```c
// Log-Level setzen
esp_log_level_set("wifi_manager", ESP_LOG_DEBUG);
esp_log_level_set("captive_portal", ESP_LOG_DEBUG);
esp_log_level_set("dns_server", ESP_LOG_DEBUG);

// Status-Check
ESP_LOGI("app", "WiFi Status: %d", wifi_manager_get_status());
ESP_LOGI("app", "Has Credentials: %s", 
         wifi_manager_has_credentials() ? "Yes" : "No");
```

## 📊 Speicherverbrauch

- **Flash**: ~60 KB (Code + HTML)
- **RAM**: ~25 KB (laufend)
- **Stack**: 8 KB (HTTP Server Task)

## 🔄 Migration von IDF 4.x zu 5.x

Der WiFi Manager ist für beide Versionen kompatibel. Keine Änderungen nötig!

## ⚠️ Troubleshooting

### Problem: Captive Portal öffnet nicht automatisch

**Lösung**: 
- DNS-Server läuft korrekt
- Prüfe Firewall-Einstellungen auf Smartphone
- Manuell zu `http://192.168.4.1` navigieren

### Problem: WiFi verbindet nicht

**Lösung**:
```c
// Erhöhe Retry-Versuche
config.max_retry_attempts = 10;

// Prüfe Logs
esp_log_level_set("wifi", ESP_LOG_DEBUG);
```

### Problem: Zu wenig Speicher

**Lösung**:
```c
// In sdkconfig oder menuconfig:
// CONFIG_HTTPD_MAX_URI_HANDLERS=16
// CONFIG_HTTPD_MAX_REQ_HDR_LEN=512
```

## 🎓 Beispiele

Siehe `main/main.c` für ein vollständiges Beispiel.

## 📚 API-Referenz

Siehe `wifi_manager.h` für vollständige API-Dokumentation.

## 🤝 Integration mit anderen Komponenten

```c
// Nach WiFi-Verbindung MQTT starten
void wifi_callback(wifi_manager_event_t event, void *data)
{
    if (event == WIFI_MANAGER_EVENT_STA_CONNECTED) {
        mqtt_app_start();
        http_client_start();
        // etc.
    }
}
```

## 📈 Zukunft

Geplante Features:
- [ ] BLE Provisioning als Alternative
- [ ] WPS-Support
- [ ] Multi-Network-Support (Fallback-Netzwerke)
- [ ] Erweiterte Netzwerk-Diagnose
- [ ] MQTT-basierte Konfiguration
