# ESP32 EDC Toolkit (compatible with idf v4.x ONLY)

## Description
ESP32 specific CPP toolkit used by EDC Electronic Design Chemnitz GmbH. Provides CPP classes for all components shared between different ESP32-Projects

Note:
> Feel free to change and extend this toolkit, but please use your own development branch and do
> extensive testing before merging into master! **Thank you!**

# Usage

 - Create a `components` directory in the project directory
 - add the ESP32 EDC Toolkit as submodule into `components/esp32-edc-toolkit` directory

## Components
The following folders defines the structure of the toolkit.

### FreeRTOS
Here are the main functions to use the async task and timer functionality of the FreeRTOS stack.

**Supported are:**
- general FreeRTOS handling
- Tasks
- Timer

### Networking
Here are classes included to manage the network interface from the physical connection (Ethernet, WiFi) up to the software defined base protocol (Socket).

**Note:** Check out the README.md in the sub-folder to use the network interface correctly!

### Protocols
This is the folder for all protocols which based of a physical layer (socket, uart, spi, ...).

**Supported are:**
- HTTP (Client and Server)
- DNS (mDNS, DNSlookup)
- Modbus (General-layer, and TCP-specific)
- Websocket
- MQTT, Azure, OpcUA (in work)

### Peripherals
This is the place for all chip specific functions and protocols.

**Supported are:**
- GPIO, I2C, UART, ADC
- SD-Card (as well as SPI file system 'SPIFFS')
- NVS (EEPROM)
- DeepSleep functions
- System functions (reboot, ...)

### Utility
Here are place for general classes. An equivalent name for this folder can be e.g. "stuff"...

**What you can find here:**
- JSON, JSON-RPC
- Memory debugging
- FileSystem
- GeneralUtils

### Modules
Classes for extern addable modules are here.

**Supported are:**
- BQ32000 (RTC)
- SIM900 (GSM)
