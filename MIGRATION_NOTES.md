# SmartLove - ESP-IDF 5.x Migration Guide

## Aktuelle Version
- ESP-IDF: v4.4.7
- Target: ESP32

## Ziel-Version
- ESP-IDF: v5.3+ (neueste LTS)
- Target: ESP32 (kann später auf ESP32-S3/C3/etc. erweitert werden)

## Migrations-Schritte

### 1. FreeRTOS Include-Änderungen
**IDF 4.x:**
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
```

**IDF 5.x:**
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
```
✅ Keine Änderung nötig für diese Includes

### 2. Kconfig-Optionen
Einige Kconfig-Optionen wurden umbenannt oder verschoben:
- `CONFIG_FREERTOS_HZ` → `CONFIG_FREERTOS_HZ` (gleich geblieben)
- Neue Optionen für Thread-Safety und Performance

### 3. GPIO API (falls später verwendet)
**IDF 4.x:**
```c
gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_2, 1);
```

**IDF 5.x:**
```c
// Gleich geblieben, aber neue Funktionen verfügbar
gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_2, 1);
```

### 4. ADC API (falls später verwendet)
**IDF 4.x:**
```c
adc1_config_width(ADC_WIDTH_BIT_12);
adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
int val = adc1_get_raw(ADC1_CHANNEL_0);
```

**IDF 5.x:**
```c
#include "esp_adc/adc_oneshot.h"
adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config = {
    .unit_id = ADC_UNIT_1,
};
adc_oneshot_new_unit(&init_config, &adc1_handle);

adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_11,
};
adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);

int val;
adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &val);
```

### 5. WiFi API Änderungen
- Event-Handler-Registrierung wurde vereinfacht
- Neue WiFi-Provisioning-Optionen

### 6. Komponenten-Struktur
In IDF 5.x können Komponenten besser organisiert werden:
```
components/
  └── my_component/
      ├── CMakeLists.txt
      ├── include/
      │   └── my_component.h
      └── my_component.c
```

## Build-System Änderungen

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)

# Target sollte vor include() gesetzt werden
set(IDF_TARGET esp32)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smartlove)
```

### sdkconfig Änderungen
Einige alte Kconfig-Optionen wurden entfernt oder umbenannt. Bei der Migration:
```bash
# Clean build empfohlen
rm -rf build sdkconfig
idf.py set-target esp32
idf.py menuconfig  # Neue Konfiguration erstellen
```

## Kompatibilitätsprüfung

### Vor der Migration prüfen:
1. ✅ Code kompiliert mit IDF 4.4.7
2. ⏳ Alle verwendeten APIs dokumentieren
3. ⏳ Externe Bibliotheken auf IDF 5.x Kompatibilität prüfen

### Nach der Migration testen:
1. ⏳ Build erfolgreich
2. ⏳ Flash erfolgreich
3. ⏳ Funktionstest auf Hardware

## Deprecated Features (vermeiden)

### IDF 4.x Features die in IDF 5.x entfernt wurden:
- `tcpip_adapter_*` → Verwende `esp_netif_*` (bereits seit 4.x deprecated)
- Alte SPI/I2C Master APIs → Neue Driver-API verwenden
- `esp_adc_cal_*` → Neue ADC API verwenden

## Best Practices für IDF 5.x

1. **Komponentenbasierte Architektur**
   - Eigene Komponenten für Modularity erstellen
   - Klare Abhängigkeiten definieren

2. **Moderne C-Standards**
   - C11/C17 Features nutzen
   - Type-Safety beachten

3. **Error Handling**
   ```c
   esp_err_t ret = some_function();
   if (ret != ESP_OK) {
       ESP_LOGE(TAG, "Error: %s", esp_err_to_name(ret));
       return ret;
   }
   ```

4. **Memory Management**
   - Heap-Caps für spezifische Memory-Regionen nutzen
   - Memory-Leaks vermeiden (valgrind-ähnliche Tools)

## Resources

- [ESP-IDF Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/index.html)
- [ESP-IDF v5.3 Documentation](https://docs.espressif.com/projects/esp-idf/en/v5.3/)
- [Breaking Changes](https://github.com/espressif/esp-idf/blob/master/CHANGELOG.md)

## Timeline

- [x] Projekt-Setup mit IDF 4.4.7
- [ ] Code-Review und Kompatibilitätsprüfung
- [ ] IDF 5.3 Installation
- [ ] Code-Migration
- [ ] Testing und Validierung
- [ ] Produktiv-Deployment
