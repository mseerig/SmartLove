#ifndef _SEN5X_HPP_
#define _SEN5X_HPP_
//#include <SensirionCore.h>
#include <stdint.h>
#include "I2CArbiter.hpp"
#include "FreeRTOS.hpp"
#include "esp_log.h"
#include <string.h>
#define SEN5x_ADDRESS 0x69
#define POLYNOMIAL 0x131
#define CHECKSUM_ERROR 0

typedef enum
{
    SEN_START_MEASUREMENT = 0x0021,     // Start Measurement
    SEN_START_MEASUREMENT_RHT = 0X0037, // start Measurement RHT
    SEN_STOP_MEASUREMENT = 0x0104,      // Stop Measurement
    SEN_READ_DATA_READY_FLAG = 0x0202,  // data ready
    SEN_READ_MEASURED_VALUES = 0x03C4,  // Read measured values

    SEN_TEMPERATURE_COMPENSATION_PARAMETERS = 0x60B2,
    SEN_WARM_START_PARAMETERS = 0x60C6,
    SEN_VOC_ALGORITHM_TUNING_PARAMETERS = 0x60D0,
    SEN_NOX_ALGORITHM_TUNING_PARAMETERS = 0x60E1,
    SEN_RHT_ACCELERATION_MODE = 0x60F7,
    SEN_VOC_ALGORITHM_STATE = 0x6181,
    SEN_START_FAN_CLEANING = 0x5607, // start fan cleaning
    SEN_AUTO_CLEANING_INTERVAL = 0x8004,
    SEN_PRODUCT_NAME = 0xD014,  // product name
    SEN_SERIAL_NUMBER = 0xD033, // serial number
    SEN_FIRMWARE_VERSION = 0xD100,
    SEN_READ_DEVICE_STATUS = 0xD206, // Read device status
    SEN_CLEAR_DEVICE_STATUS = 0xD210,
    SEN_DEVICE_RESET = 0xD304, // device reset

    //...
} sen5x_cmd_t;

// measured pramater by sensor
typedef struct
{
    float pm1p0;
    float pm2p5;
    float pm4p0;
    float pm10p0;
    float humidity;
    float temperature;
    int16_t voc;
    int16_t nox;
} sen5x_data_t;

typedef struct
{
    uint8_t firmwareVersion;
    std::string productName;
    std::string serialNumber;

} sen5x_info_t;

typedef struct
{
    bool speed_fan_error;  // Fan speed is too high or too low.
    bool fan_cleaning;     // true Active during the automatic cleaning procedure of the fan
    bool gas_sensor_error; // true Gas sensor error
    bool rht_comuni_error; // true Error in internal communication with rht sensor
    bool laser_error;      // true laser is switched on and current is out of range
    bool fan_error;        // Fan is switched on, but the measured fan speed is 0 RPM.
    //..
} sen5x_status_t;

class SEN5X
{

private:
    I2CArbiter & m_i2c;
    sen5x_data_t m_data;
    sen5x_info_t m_info;
    sen5x_status_t m_status;

    esp_err_t stratMeasurement();
    esp_err_t stopMeasurement();
    esp_err_t startMeasurementRht();
    esp_err_t readDataReady(bool &dataReady);

    esp_err_t readFirmwareVersion(uint8_t &firmwareVersion);
    esp_err_t getProductName(std::string &productName);
    esp_err_t getSerialNumber(std::string &serialNumber);

    esp_err_t getDeviceStatus(uint32_t &deviceStatus);

    esp_err_t sendCommand(uint16_t cmd);
    esp_err_t checkCRC(uint8_t data[], uint8_t len, bool useZeroTermination = false);
    esp_err_t readCommand(uint8_t buffer[], uint8_t len);

public:
    SEN5X(I2CArbiter &i2c);
    ~SEN5X();
    void run(); // not used

    sen5x_data_t getData() { return m_data; }
    sen5x_info_t getDeviceInfo() { return m_info; }
    sen5x_status_t getStatus() { return m_status; }

    esp_err_t readDeviceStatus();
    esp_err_t doMeasurement(); // fill private m_data
    esp_err_t startFanCleaning();
    esp_err_t readDeviceInfo();
    esp_err_t deviceReset();
};

#endif