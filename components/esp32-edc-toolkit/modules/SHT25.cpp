/**
 * @file SHT25.cpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "SHT25.hpp"
#define LOGTAG "SHT25"

/**
 * @brief Construct a new SHT25::SHT25 object
 * 
 * @param[in] i2c I2CArbiter instance
 */
SHT25::SHT25(I2CArbiter & i2c):
    m_i2c(i2c)
{
    //disableHeater();
}

/**
 * @brief Destroy the SHT25::SHT25 object
 * 
 */
SHT25::~SHT25(){

}

/**
 * @brief function used to perform a soft reset of the sensor
 * 
 * @return esp_err_t i2c error code
 */
esp_err_t SHT25::resetSensor(){
    esp_err_t ret;

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_SOFT_RESET, true);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();
    //if COM was succesfull wait for reset of the SHT25
    if(ret == ESP_OK) FreeRTOS::sleep(15);
    return ret;
}

/**
 * @brief function used to select the resolution of the temperature and humidity sensor; can only be one of sht25_res_t
 * 
 * @param[in] resCombination resolution combination to be selected
 * @return esp_err_t i2c error code
 */
esp_err_t SHT25::setResCombination(sht25_res_t resCombination){
    esp_err_t ret;
    uint8_t buffer;

    //TODO: maybe widen the delay to reduce the number of retries (10ms every retry)
    switch(resCombination){
        case 0x00:                 //RH 12bit, Temp 14bit
            m_tempDelay = 85;
            m_rhDelay = 29;
            break;
        case 0x01:                 //RH 8bit, Temp 12bit
            m_tempDelay = 22;
            m_rhDelay = 4;
            break;
        case 0x80:                 //RH 10bit, Temp 13bit
            m_tempDelay = 43;
            m_rhDelay = 9;
            break;
        case 0x81:                 //RH 11bit, Temp 11bit
            m_tempDelay = 11;
            m_rhDelay = 15;
            break;
        default:
            return ESP_FAIL;
    }

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_READ_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (ret == ESP_OK) ret = i2c_master_read_byte(cmd, &buffer, I2C_MASTER_LAST_NACK);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();	//release bus anyway

    buffer &= 0x7E; //reset bit 0 and 7
    buffer |= resCombination; //set new bits

    ret = m_i2c.acquireBus();
    cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_WRITE_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, buffer, true);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
	m_i2c.releaseBus();	//release bus anyway

	return ret;
}

/**
 * @brief function used to enable the on chip heater of the sht25
 * 
 * @return esp_err_t i2c error code
 */
esp_err_t SHT25::enableHeater(){
    esp_err_t ret;
    uint8_t buffer;

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_READ_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (ret == ESP_OK) ret = i2c_master_read_byte(cmd, &buffer, I2C_MASTER_LAST_NACK);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();	//release bus anyway

    buffer |= 0b100; //set bit 2

    ret = m_i2c.acquireBus();
    cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_WRITE_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, buffer, true);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
	m_i2c.releaseBus();	//release bus anyway

	return ret;
}

/**
 * @brief function used to disable the on chip heater of the sht25
 * 
 * @return esp_err_t i2c error code
 */
esp_err_t SHT25::disableHeater(){

    esp_err_t ret;
    uint8_t buffer;

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_READ_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (ret == ESP_OK) ret = i2c_master_read_byte(cmd, &buffer, I2C_MASTER_LAST_NACK);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();	//release bus anyway

    buffer &= 0b11111011; //reset bit 2

    ret = m_i2c.acquireBus();
    cmd = i2c_cmd_link_create();

    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_WRITE_USER_REG, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, buffer, true);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);
	m_i2c.releaseBus();	//release bus anyway

	return ret;
}

/**
 * @brief function used to trigger a temperature measurement of the sht25 and retrieve the result; blocks for at least
 *        m_tempDelay (max. conversion time in this setting from datasheet) but up to m_tempDelay + 100ms
 * 
 * @param[out] temperature calculated temperature in degrees celsius
 * @return esp_err_t i2c error code or FAIL if wrong measurement
 */
esp_err_t SHT25::getTemperature(float &temperature){
    esp_err_t ret;
    uint8_t buffer [3] = {0x00,0x02,0x00};

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_READ_TEMP_NO_HOLD, true);
    FreeRTOS::sleep(1);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();	//release bus anyway

    if (ret == ESP_OK) FreeRTOS::sleep(m_tempDelay);

    esp_err_t loop_ret = ESP_FAIL;
    uint8_t retry_count = 0;

    //try to retrieve the temperature data; if answer is NACK or timeout; retry
    while((retry_count < 10) && ((loop_ret == ESP_FAIL)||(loop_ret == ESP_ERR_TIMEOUT))){
        loop_ret = m_i2c.acquireBus();
        cmd = i2c_cmd_link_create();
        if (loop_ret == ESP_OK) loop_ret = i2c_master_start(cmd);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_READ, true);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_read(cmd, buffer, 3, I2C_MASTER_LAST_NACK);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_stop(cmd);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 5 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        m_i2c.releaseBus();	//release bus anyway
        retry_count++;
        FreeRTOS::sleep(5);
    }

    ret = loop_ret;

    //check if Value is temperature
    if (buffer[1] & 1<<1) {
        return ESP_FAIL;
    }

    if (ret == ESP_OK) {
        temperature = (float)((unsigned int)buffer[0]*((int)1<<8) + (unsigned int)(buffer[1]&(0xFC)));
        temperature = 175.72*temperature/((int)1<<16) -46.85;
    }

    return ret;
}

/**
 * @brief function used to trigger a humidity measurement of the sht25 and retrieve the result; blocks for at least
 *        m_rhDelay (max. conversion time in this setting from datasheet) but up to m_rhDelay + 100ms
 * 
 * @param[out] humidity calculated humidity in %RH
 * @return esp_err_t i2c error code or FAIL if wrong measurement
 */
esp_err_t SHT25::getHumidity(float &humidity){
    esp_err_t ret;
    uint8_t buffer [3] = {0x00,0x00,0x00};

    ret = m_i2c.acquireBus();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (ret == ESP_OK) ret = i2c_master_start(cmd);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (ret == ESP_OK) ret = i2c_master_write_byte(cmd, SHT25_READ_HUMI_NO_HOLD, true);
    FreeRTOS::sleep(1);
    if (ret == ESP_OK) ret = i2c_master_stop(cmd);
    if (ret == ESP_OK) ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 5 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    m_i2c.releaseBus();	//release bus anyway

    if (ret == ESP_OK) FreeRTOS::sleep(m_rhDelay);

    esp_err_t loop_ret = ESP_FAIL;
    uint8_t retry_count = 0;

    //try to retrieve the humidity data; if answer is NACK or timeout; retry
    while((retry_count < 10) && ((loop_ret == ESP_FAIL)||(loop_ret == ESP_ERR_TIMEOUT))){
        loop_ret = m_i2c.acquireBus();
        cmd = i2c_cmd_link_create();
        if (loop_ret == ESP_OK) loop_ret = i2c_master_start(cmd);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_write_byte(cmd, (SHT25_ADDRESS << 1) | I2C_MASTER_READ, true);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_read(cmd, buffer, 3, I2C_MASTER_LAST_NACK);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_stop(cmd);
        if (loop_ret == ESP_OK) loop_ret = i2c_master_cmd_begin(m_i2c.getPortNum(), cmd, 100 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        m_i2c.releaseBus();	//release bus anyway
        retry_count++;
        FreeRTOS::sleep(5);
    }

    ret = loop_ret;

    //check if Value is humidity
    if (!(buffer[1] & 1<<1)){
        return ESP_FAIL;
    }

    if (ret == ESP_OK) {
        humidity = (float)((unsigned int)buffer[0]*((int)1<<8) + (unsigned int)(buffer[1]&(0xFC)));
        humidity = 125.0*humidity/((int)1<<16) -6.0;
    }

    return ret;
}

//TODO: maybe use CRC-8 to validate reads (https://stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c)