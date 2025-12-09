/**
 * @file SHT25.hpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "I2CArbiter.hpp"
#include "FreeRTOS.hpp"
#include "esp_log.h"

#define SHT25_ADDRESS 0x40

typedef enum {
    SHT25_READ_TEMP_NO_HOLD = 0xF3,
    SHT25_READ_TEMP_HOLD = 0xE3,
    SHT25_READ_HUMI_NO_HOLD = 0xF5,
    SHT25_READ_HUMI_HOLD = 0xE5,
    SHT25_WRITE_USER_REG = 0xE6,
    SHT25_READ_USER_REG = 0xE7,
    SHT25_SOFT_RESET = 0xFE
} sht25_cmd_t;

//resolution combinations SHT25_RES_RH_TEMP
typedef enum {
    SHT25_RES_12_14BIT = 0x00,
    SHT25_RES_8_12BIT = 0x01,
    SHT25_RES_10_13BIT = 0x80,
    SHT25_RES_11_11BIT = 0x81
} sht25_res_t;

class SHT25
{
    public:
        SHT25(I2CArbiter &i2c);
        ~SHT25();
        esp_err_t resetSensor();
        esp_err_t setResCombination(sht25_res_t resCombination);
        esp_err_t enableHeater();
        esp_err_t disableHeater();
        esp_err_t getTemperature(float &temperature);
        esp_err_t getHumidity(float &humidity);
    private:
        I2CArbiter& m_i2c;

        uint8_t     m_tempDelay{85};
        uint8_t     m_rhDelay{29};
};
