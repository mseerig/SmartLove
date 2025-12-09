/**
 * @file MCP4802.cpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "MCP4802.hpp"

#define LOGTAG "MCP4802"

/**
 * @brief Construct a new MCP4802::MCP4802 object
 * 
 */
MCP4802::MCP4802(gpio_num_t cs, spi_host_device_t spi_host){
    esp_err_t ret;

    spi_device_interface_config_t devcfg = {
        .command_bits = 16,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 3,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 100*1000,
        .input_delay_ns = 0,
        .spics_io_num = cs,
        .flags = 0,
        .queue_size = 7,
        .pre_cb= NULL,
        .post_cb= NULL,
    };

    //Attach the Flash to the SPI bus
    ret=spi_bus_add_device(spi_host, &devcfg, &m_spi);
    ESP_ERROR_CHECK(ret);
}

/**
 * @brief Destroy the MCP4802::MCP4802 object
 * 
 */
MCP4802::~MCP4802()
{
    spi_bus_remove_device(m_spi);
}

/**
 * @brief function used to set the voltage of the DAC A
 * 
 * @param voltage 0x00 - 0xFF voltage value
 * @return esp_err_t spi error code
 */
esp_err_t MCP4802::setDACAVoltage(uint8_t voltage, mcp4802_output_gain_t gain)
{
    esp_err_t ret;
    uint16_t command;

    if (voltage == 0){                              //if voltage == 0 shutdown the DAC channel
        command = 0;
        command |= (gain << 13);
    }else{                                          //if voltage != 0 set the voltage and disable shutdown
        command = 0;
        command |= (1 << 12); // disable shutdown
        command |= (gain << 13); // set the gain
        command |= voltage << 4; // set the voltage
    }

    spi_transaction_t setDACAConf{
        .flags = 0,
        .cmd = command,
        .addr = 0,
        .length =  0,
        .rxlength = 0,  
        .user = NULL,
        .tx_buffer = NULL,
        .rx_buffer = NULL,
    };

    ret = spi_device_polling_transmit(m_spi, &setDACAConf);

    if(ret != ESP_OK)
        ESP_LOGE(LOGTAG, "Writing DAC A failed: %s", esp_err_to_name(ret));

    return ret;
}

/**
 * @brief function used to set the voltage of the DAC B
 * 
 * @param voltage 0x00 - 0xFF voltage value
 * @return esp_err_t spi error code
 */
esp_err_t MCP4802::setDACBVoltage(uint8_t voltage, mcp4802_output_gain_t gain)
{
    esp_err_t ret;
    uint16_t command;

    if (voltage == 0){                              //if voltage == 0 shutdown the DAC channel
        command = 0b1000000000000000;
        command |= (gain << 13);
    }else{                                          //if voltage != 0 set the voltage and disable shutdown
        command = 0b1000000000000000;
        command |= (1 << 12); // disable shutdown
        command |= (gain << 13); // set the gain
        command |= voltage << 4; // set the voltage
    }

    spi_transaction_t setDACAConf{
        .flags = 0,
        .cmd = command,
        .addr = 0,
        .length =  0,
        .rxlength = 0,  
        .user = NULL,
        .tx_buffer = NULL,
        .rx_buffer = NULL,
    };

    ret = spi_device_polling_transmit(m_spi, &setDACAConf);

    if(ret != ESP_OK)
        ESP_LOGE(LOGTAG, "Writing DAC A failed: %s", esp_err_to_name(ret));

    return ret;
}