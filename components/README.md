# SmartLove Components

This directory contains custom components for the SmartLove project.

## Component Structure

Each component should follow this structure:
```
components/
  └── component_name/
      ├── CMakeLists.txt
      ├── Kconfig (optional)
      ├── include/
      │   └── component_name.h
      └── component_name.c
```

## Example Component

### CMakeLists.txt
```cmake
idf_component_register(
    SRCS "component_name.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer
)
```

### Future Components (Planned)

- **sensors/** - Sensor drivers and data acquisition
- **wifi_manager/** - WiFi connection and management
- **bluetooth/** - BLE communication
- **storage/** - Non-volatile storage management
- **communication/** - Data protocols and APIs
- **security/** - Encryption and secure communication

## IDF 5.x Compatibility

All components should be designed to work with both IDF 4.x and 5.x where possible.
For features specific to IDF 5.x, use preprocessor checks:

```c
#include "esp_idf_version.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    // IDF 5.x specific code
#else
    // IDF 4.x fallback
#endif
```
