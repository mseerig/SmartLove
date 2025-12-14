# Known Issues

## Minor Bugs (Niedrige Priorität)

### 1. WiFi Scan zeigt keine Ergebnisse im Web UI

**Status:** 🐛 Offen  
**Priorität:** Minor  
**Entdeckt:** 14. Dezember 2025

**Beschreibung:**
- Das Captive Portal öffnet sich korrekt
- Die Web-Oberfläche wird angezeigt
- Der WiFi-Scan wird ausgeführt, aber keine Netzwerke werden im UI angezeigt
- Möglicherweise ein Problem mit:
  - `wifi_manager_scan()` Funktion
  - JSON-Serialisierung der Scan-Ergebnisse
  - HTTP-Endpoint `/scan` Response-Format
  - Frontend JavaScript Parsing

**Betroffene Dateien:**
- `components/wifi_manager/wifi_manager.c` (scan Logik)
- `components/wifi_manager/captive_portal.c` (HTTP `/scan` endpoint)

**Debugging-Schritte:**
```bash
# Log-Level erhöhen
esp_log_level_set("wifi_manager", ESP_LOG_DEBUG);
esp_log_level_set("captive_portal", ESP_LOG_DEBUG);

# Prüfen ob Scan-Ergebnisse ankommen
# Prüfen ob HTTP Response korrekt formatiert ist
```

**Temporäre Workaround:**
- Manuell SSID und Passwort eingeben (funktioniert)

---

### 2. Komischer Prefix vor "SmartLove" AP-Name

**Status:** 🐛 Offen  
**Priorität:** Minor  
**Entdeckt:** 14. Dezember 2025

**Beschreibung:**
- Der Access Point Name zeigt einen unerwarteten Prefix
- Statt "SmartLove-XXXX" erscheint etwas wie "�SmartLove-XXXX" oder ähnliches
- Sieht aus wie ein falscher Parse oder Buffer-Problem
- Wahrscheinlich ein String-Formatierungs- oder Encoding-Problem

**Betroffene Dateien:**
- `main/main.c` (AP-Name wird mit `snprintf()` erstellt)
- `components/wifi_manager/wifi_manager.c` (AP-Konfiguration)

**Vermutete Ursache:**
```c
// In main.c, Zeile ~42:
snprintf(config.ap_ssid, sizeof(config.ap_ssid), "SmartLove-%04X", 
         esp_random() & 0xFFFF);
```

Mögliche Probleme:
- Buffer nicht null-terminiert
- Format-String Problem
- `esp_random()` wird zu früh aufgerufen (vor Initialisierung)
- Memory corruption / uninitialisierter Buffer

**Debugging-Schritte:**
```c
// SSID vor und nach snprintf loggen
ESP_LOGI("main", "SSID buffer before: [%s]", config.ap_ssid);
snprintf(config.ap_ssid, sizeof(config.ap_ssid), "SmartLove-%04X", 
         esp_random() & 0xFFFF);
ESP_LOGI("main", "SSID after: [%s]", config.ap_ssid);
ESP_LOG_BUFFER_HEX("main", config.ap_ssid, sizeof(config.ap_ssid));
```

**Mögliche Lösungen:**
1. Buffer vorher mit `memset()` nullen
2. Direkt einen festen String verwenden für Tests
3. `esp_random()` später aufrufen
4. Sicherstellen dass `wifi_manager_get_default_config()` den Buffer korrekt initialisiert

**Temporäre Workaround:**
- Funktionalität ist nicht beeinträchtigt, nur kosmetisch

---

## Notizen

- Beide Bugs beeinträchtigen die Funktionalität nicht kritisch
- WiFi-Verbindung funktioniert (manuelle SSID-Eingabe)
- Credentials werden korrekt gespeichert
- Auto-Connect funktioniert nach Neustart
- Captive Portal wird automatisch erkannt

## Nächste Schritte (für später)

1. WiFi Scan Debugging
   - [ ] Log-Ausgaben in `wifi_manager_scan()` hinzufügen
   - [ ] HTTP Response vom `/scan` Endpoint prüfen
   - [ ] JavaScript Console im Browser checken
   - [ ] Scan-Ergebnisse Array prüfen

2. AP-Name Prefix Fix
   - [ ] Buffer-Initialisierung prüfen
   - [ ] SSID hex-dump loggen
   - [ ] Alternative Formatierung testen
   - [ ] `esp_random()` Timing prüfen

3. Allgemeine Verbesserungen
   - [ ] Error-Handling robuster machen
   - [ ] Mehr Debug-Logs hinzufügen
   - [ ] Unit-Tests für String-Formatierung
   - [ ] Integration-Tests für Web UI
