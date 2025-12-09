# Documentation

These are Scripts, used to perform the whole dual build process. 

## Requirements

- esp32-scripts is submodule of root project
- Files are correctly configured in root project:
    - ``partitions_debug.csv``
    - ``partitions_release.csv``
    - ``sdkconfig_debug.defaults`` Note: Needs ``partitions_debug.csv``
    - ``sdkconfig_release.defaults`` Note: Needs ``partitions_release.csv``
    - ``projectConfig.csv`` see Example below.

### Usage of projectConfig.csv

Example:

``` 
######################################################
###############  Projektkonfiguration  ###############
######################################################

### Flash Einstellungen ###
target,                 debug
chip,                   esp32s3
flash_port,             COM3
baudrate,               2000000

### Projekt-Files ###

definitionshpp,         main/Definitions.hpp
keyfile,               "edc_secure_boot_signing_key.pem"
flash_encryption_key,  "edc_flash_encryption_key.bin"
webfrontend,            webfrontend
esp_tool_dir,           esp32-scripts/esptool
esp_idf_path,           /opt/esp/idf
```

`esp_idf_path` is set to default `/opt/esp/idf` used by the ci docker.<br>
`definitionshpp`, `keyfile`, `webfrontend` and `esp_tool_dir` are used to specify the location corresponding to the project root dir, where the scripts are should be used.

## Usage

**Example:** ```python .\esp32-scripts\esp32-main-builder.py build update target=release flash_port=COM23 esp_idf_path=C:/Users/marcel.seerig/.espressif/frameworks/esp-idf-v4.4.1```

The used terminal scope should know the idf.py file to use build and debug commands. Flashing, or encrypting a device work's with out idf.py.

Linux: ``python esp32-main-builder.py <COMMAND> <METHOD> <OVERRIDE-ARGS>``<br>
Windows: ``python esp32-main-builder.py <COMMAND> <METHOD> <OVERRIDE-ARGS>``

Use debug target for development and release for ci builds.

`<OVERRIDE-ARGS>` can be usees to override `key=value` pairs from the projectConfig.csv like `flash_port=COM43`. 


### Examples

``` 
build app
build data
build update
build clean
build fullclean

flash app
flash data
flash update
flash erase

prepare menuconfig (NOTE: Will not effect build config!!)
prepare encrypt-chip

read <partitionName>

debug monitor
```

## How to add executable scripts?

- add submodule here and define project specific script
- change file permissions to 755 (executable)!

See: https://stackoverflow.com/questions/21691202/how-to-create-file-execute-mode-permissions-in-git-on-windows

