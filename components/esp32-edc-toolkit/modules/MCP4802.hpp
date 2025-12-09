/**
 * @file MCP4802.hpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "Definitions.hpp"

typedef enum{
    GAIN_TWO_VREF = 0b00,   //VOUT = 2 * VREF * (D/256)
    GAIN_ONE_VREF = 0b01,   //VOUT = 1 * VREF * (D/256)
}mcp4802_output_gain_t;

class MCP4802 {
    public:
        MCP4802(gpio_num_t cs, spi_host_device_t spi_host);
        ~MCP4802();

        esp_err_t setDACAVoltage(uint8_t voltage, mcp4802_output_gain_t gain = GAIN_TWO_VREF);
        esp_err_t setDACBVoltage(uint8_t voltage, mcp4802_output_gain_t gain = GAIN_TWO_VREF);

    private:
        spi_device_handle_t m_spi;
};