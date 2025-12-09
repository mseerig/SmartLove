/**
 * @file SEN5x.cpp
 * @author Hooman (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "SEN5x.hpp"
#include "esp_log.h"
#include "GPIO.hpp"
#include <FreeRTOS.hpp>
#include <math.h>

#include <cstring>
#include <string.h>
#define UINT_INVALID 0xFFFF
#define INT_INVALID 0x7FFF

static char LOGTAG[] = "SEN5X";

/**
 * @brief Construct a new SEN5x::SEN5x object
 *
 * @param[in] i2c I2CArbiter instance
 */

SEN5X::SEN5X(I2CArbiter &i2c) : m_i2c(i2c)
{
    ESP_LOGI(LOGTAG, "Starting...");
}
// read device info in to m_info
esp_err_t SEN5X::readDeviceInfo()
{
    esp_err_t ret;

    readFirmwareVersion(m_info.firmwareVersion);

    ESP_LOGD(LOGTAG, " reaDeviceInfo: firmware: %d", m_info.firmwareVersion);

    getProductName(m_info.productName);

    ESP_LOGD(LOGTAG, " reaDeviceInfo: productName: %s", m_info.productName.c_str());

    getSerialNumber(m_info.serialNumber);

    ESP_LOGD(LOGTAG, " reaDeviceInfo: serialNumber: %s", m_info.serialNumber.c_str());

    ret = 0;
    return ret;
}

/**
 * @brief Destroy the SEN5X::SEN5X object
 *
 */

SEN5X::~SEN5X()
{
}

void SEN5X::run()
{
    ESP_LOGD(LOGTAG, "loop");
}

/**
 * @brief function used to perform a start_measurement of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::stratMeasurement()
{
    esp_err_t ret;
    ret = sendCommand(SEN_START_MEASUREMENT);
    // if COM was succesfull wait for start_measurement of the Sen5x
    if (ret == ESP_OK)
        FreeRTOS::sleep(50);
    return ret;
}

/**
 * @brief function used to perform a start_measurement_RHT of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::startMeasurementRht()
{
    esp_err_t ret;
    ret = sendCommand(SEN_START_MEASUREMENT_RHT);

    // if COM was succesfull wait for start_measurement RHT of the Sen5x
    if (ret == ESP_OK)
        FreeRTOS::sleep(50);
    return ret;
}

/**
 * @brief function used to perform a stop_measurement of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::stopMeasurement()
{
    esp_err_t ret;
    ret = sendCommand(SEN_STOP_MEASUREMENT);
    if (ret == ESP_OK)
        FreeRTOS::sleep(200);
    return ret;
}

/**
 * @brief function used to read frimversion of sensor
 *
 * @param[in] getFrimVersion take frimversion of sensor
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::readFirmwareVersion(uint8_t &firmwareVersion)
{
    esp_err_t ret;
    uint8_t buffer[6]; // increased number of byte of buffer
    ret = sendCommand(SEN_FIRMWARE_VERSION);
    ret = readCommand(buffer, 6);

    ESP_LOGD(LOGTAG, "firmwareVersion: %d", buffer[0]);
    firmwareVersion = buffer[0];

    ret = checkCRC(buffer, 6, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LOGTAG, "CRC error");
        return ret;
    }

    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to Read data ready of sesnor
 *
 * @param[in] dataReady take dataReady
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::readDataReady(bool &dataReady)
{
    esp_err_t ret;
    uint8_t buffer[3];
    ret = sendCommand(SEN_READ_DATA_READY_FLAG);
    ret = readCommand(buffer, 3);
    ret = checkCRC(buffer, 3, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LOGTAG, "CRC error");
        return ret;
    }

    ESP_LOGD(LOGTAG, "DataReady: %d", buffer[1]);
    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to Read productname of the SEN5x and retrieve the result
 *
 * @param[in] productName take productName
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::getProductName(std::string &productName)
{
    esp_err_t ret;
    uint8_t buffer[48];

    ret = sendCommand(SEN_PRODUCT_NAME);
    ret = readCommand(buffer, 48);

    productName = "";

    ret = checkCRC(buffer, 48, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LOGTAG, "CRC error");
        return ret;
    }

    for (int i = 0; i < (sizeof(buffer) / sizeof(buffer[0])); i++)
    {
        if ((i + 1) % 3 != 0)
        { // without checksum
            productName += buffer[i];
        }
    }

    ESP_LOGD(LOGTAG, "ProductName: %s", productName.c_str());
    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to read measured value of the SEN5x and retrieve the result
 *
 * @param[in] m_data measured pramater by sensor.
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::doMeasurement()
{
    esp_err_t ret = stratMeasurement();
    if(ret != ESP_OK) return ret;

    uint8_t buffer[24];

    ret = sendCommand(SEN_READ_MEASURED_VALUES);
    if(ret != ESP_OK) return ret;

    ret = readCommand(buffer, 24);
    if(ret != ESP_OK) return ret;

    ret = checkCRC(buffer, 24, true);
    if (ret != ESP_OK){
        ESP_LOGE(LOGTAG, "CRC error");
        ESP_LOGE(LOGTAG,"Buffer: %d,%d,%d,%d", buffer[0], buffer[1], buffer[2], buffer[3]);
        return ret;
    }

    // No erros! Just Parse
    m_data.pm1p0 = (float)((buffer[0] * (1 << 8)) + buffer[1]);
    m_data.pm1p0 = m_data.pm1p0 / 10.0;
    ESP_LOGD(LOGTAG, "pm1p0: %f", m_data.pm1p0);


    m_data.pm2p5 = (float)((buffer[3] * (1 << 8)) + buffer[4]);
    m_data.pm2p5 = m_data.pm2p5 / 10.0;
    ESP_LOGD(LOGTAG, "pm2p5: %f", m_data.pm2p5);


    m_data.pm4p0 = (float)((buffer[6] * (1 << 8)) + buffer[7]);
    m_data.pm4p0 = m_data.pm4p0 / 10.0;
    ESP_LOGD(LOGTAG, "pm4p0: %f", m_data.pm4p0);
    
    m_data.pm10p0 = (float)((buffer[9] * (1 << 8)) + buffer[10]);
    m_data.pm10p0 = m_data.pm10p0 / 10.0;
    ESP_LOGD(LOGTAG, "pm10p0: %f", m_data.pm10p0);
    
    m_data.humidity = (float)((buffer[12] * (1 << 8)) + buffer[13]);
    m_data.humidity = m_data.humidity / 100.0;
    ESP_LOGD(LOGTAG, "humidity: %f", m_data.humidity);
    
    m_data.temperature = (float)((buffer[15] * (1 << 8)) + buffer[16]);
    m_data.temperature = m_data.temperature / 200.0;
    ESP_LOGD(LOGTAG, "temperature: %f", m_data.temperature);
    
    m_data.voc = (uint16_t)((buffer[18] * (1 << 8)) + buffer[19]);
    m_data.voc = m_data.voc / 10;
    ESP_LOGD(LOGTAG, "voc: %d", m_data.voc);
    
    m_data.nox = (uint16_t)((buffer[21] * (1 << 8)) + buffer[22]);
    m_data.nox = m_data.nox / 10;
    ESP_LOGD(LOGTAG, "nox: %d", m_data.nox);

    FreeRTOS::sleep(20);
    return ret; // ESP_OK
}

/**
 * @brief function used to Read productname of the SEN5x and retrieve the result
 *
 * @param[in] serialNumber take serialNumber
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::getSerialNumber(std::string &serialNumber)
{
    esp_err_t ret;
    uint8_t buffer[48];

    ret = sendCommand(SEN_SERIAL_NUMBER);
    ret = readCommand(buffer, 48);

    ret = checkCRC(buffer, 48, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LOGTAG, "CRC error");
        return ret;
    }

    serialNumber = "";

    for (int i = 0; i < (sizeof(buffer) / sizeof(buffer[0])); i++)
    {
        if ((i + 1) % 3 != 0)
        { // without checksum
            serialNumber += buffer[i];
        }
    }

    ESP_LOGD(LOGTAG, "serial_number: %s", serialNumber.c_str());

    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to read device statuse Register of the SEN5x and retrieve the result
 *
 * @param[in] deviceStatus take deviceStatus
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::getDeviceStatus(uint32_t &deviceStatus)
{
    esp_err_t ret;

    uint8_t buffer[6];
    memset(buffer, 0, 6);

    ret = sendCommand(SEN_READ_DEVICE_STATUS);
    ret = readCommand(buffer, 6);

    ret = checkCRC(buffer, 6, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LOGTAG, "CRC error");
        return ret;
    }

    deviceStatus = (uint32_t)(buffer[0] * (1 << 24)) + (uint32_t)(buffer[1] * (1 << 16)) + (uint32_t)(buffer[3] * (1 << 8)) + (uint32_t)(buffer[4]);
    if ((deviceStatus & (0x200000)) == 0x200000)
    {
        m_status.speed_fan_error = 1;

        ESP_LOGD(LOGTAG, "  speed_fan_error");
    }

    if ((deviceStatus & (0x80000)) == 0x80000)
    {
        m_status.fan_cleaning = 1;

        ESP_LOGD(LOGTAG, "  fan_cleaning");
    }

    if ((deviceStatus & (0x80)) == 0x80)
    {
        m_status.gas_sensor_error = 1;

        ESP_LOGD(LOGTAG, "  gas_sensor_error");
    }

    if ((deviceStatus & (0x40)) == 0x40)
    {
        m_status.rht_comuni_error = 1;

        ESP_LOGD(LOGTAG, "  rht_comuni_error");
    }

    if ((deviceStatus & (0x20)) == 0x20)
    {
        m_status.laser_error = 1;

        ESP_LOGD(LOGTAG, "  laser_error");
    }

    if ((deviceStatus & (0x10)) == 0x10)
    {
        m_status.fan_error = 1;

        ESP_LOGD(LOGTAG, "  fan_error");
    }

    ESP_LOGD(LOGTAG, "  deviceStatus: %d", deviceStatus);

    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to perform a soft reset of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::deviceReset()
{
    esp_err_t ret;

    ret = sendCommand(SEN_DEVICE_RESET);

    if (ret == ESP_OK)
        FreeRTOS::sleep(100);
    return ret;
}

/**
 * @brief function used to start fan cleaning of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::startFanCleaning()
{
    esp_err_t ret;

    ret = sendCommand(SEN_START_FAN_CLEANING);

    if (ret == ESP_OK)
        FreeRTOS::sleep(20);
    return ret;
}

/**
 * @brief function used to readDeviceStatus of the sensor
 *
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::readDeviceStatus()
{
    esp_err_t ret;
    esp_err_t getDeviceStatus(uint32_t & deviceStatus);

    ret = 0;
    return ret;
}

/**
 * @brief function used to sendCommand to the sensor
 *
 *@param[in] cm take command
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::sendCommand(uint16_t cmd)
{
    esp_err_t ret;

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t hnd = i2c_cmd_link_create();
    if (ret == ESP_OK)
        ret = i2c_master_start(hnd);
    if (ret == ESP_OK)
        ret = i2c_master_write_byte(hnd, (SEN5x_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK)
        ret = i2c_master_write_byte(hnd, (cmd >> 8) & 0xFF, true);
    if (ret == ESP_OK)
        ret = i2c_master_write_byte(hnd, (cmd >> 0) & 0xFF, true);
    if (ret == ESP_OK)
        ret = i2c_master_stop(hnd);
    if (ret == ESP_OK)
        ret = i2c_master_cmd_begin(m_i2c.getPortNum(), hnd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(hnd);
    m_i2c.releaseBus();

    return ret;
}

/**
 * @brief function used to readcommand
 *
 *@param[in] bufffer take buffer
 @param[in] len take length
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::readCommand(uint8_t buffer[], uint8_t len)
{
    esp_err_t ret;

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t hnd = i2c_cmd_link_create();

    if (ret == ESP_OK)
        ret = i2c_master_start(hnd);
    if (ret == ESP_OK)
        ret = i2c_master_write_byte(hnd, (SEN5x_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (ret == ESP_OK)
        ret = i2c_master_read(hnd, buffer, len, I2C_MASTER_LAST_NACK);
    if (ret == ESP_OK)
        ret = i2c_master_stop(hnd);
    if (ret == ESP_OK)
        ret = i2c_master_cmd_begin(m_i2c.getPortNum(), hnd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(hnd);
    m_i2c.releaseBus();

    return ret;
}

/**
 * @brief function used to chechsum
 *
 *@param[in] data take data
 * @param[in] len take length
 * @param[in] useZeroTermination take useZeroTermination
 * @return esp_err_t i2c error code
 */

esp_err_t SEN5X::checkCRC(uint8_t data[], uint8_t len, bool useZeroTermination)
{
    uint8_t buffer_size = len;

    if (useZeroTermination)
    {
        for (int i = 0; i < buffer_size; i++)
        {
            if (data[i] == (uint8_t)'\0')
                buffer_size = i;
        }
    }

    for (uint8_t offset = 0; offset < buffer_size + 3; offset += 3)
    {
        //  ESP_LOGD(LOGTAG, "offset: %d", offset);

        uint8_t crc = 0xFF;
        for (int i = 0; i < 2; i++)
        {
            crc ^= data[offset + i];
            for (uint8_t bit = 8; bit > 0; --bit)
            {
                if (crc & 0x80)
                {
                    crc = (crc << 1) ^ 0x31u;
                }
                else
                {
                    crc = (crc << 1);
                }
            }
        }

        // ESP_LOGD(LOGTAG, "element %d with data %d - calculated %d", offset+2, data[offset+2], crc);
        if (crc != data[offset + 2])
            return ESP_FAIL;
    }

    return ESP_OK;
}
