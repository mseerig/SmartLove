/**
 * @file MCP47FXBXX.hpp
 * @author Erik Friedel (erik.friedel@ed-chemnitz.de)
 * @brief 8/10/12-Bit Quad/Octal DAC with NVM (EEPROM) Option
 * 
 * MCP47FX -> V: RAM; E: EEPROM
 * MCP47FXBX -> 0: 8-Bit; 1: 10-Bit; 2: 12-Bit
 * MCP47FXBXX -> 4: quad; 8: octal
 * select device type when constructing the class
 * 
 * @version 0.1
 * @date 2024-05-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef MAIN_DRIVERS_MCP47FXBXX_H_
#define MAIN_DRIVERS_MCP47FXBXX_H_

#include <stdio.h>      /* printf, scanf */
#include <string>
#include <sys/time.h>

#include <esp_log.h>
#include <esp_err.h>
#include "I2CArbiter.hpp"

//Address has only 7bit, because the "least significant bit" is reserved for ACK/NAK.
#define MCP47FXBXX_AD0_OFFSET   0x01    /*!< represents address offset, if AD0 is HIGH*/
#define MCP47FXBXX_AD1_OFFSET   0x02    /*!< represents address offset, if AD1 is HIGH*/
#define MCP47FXBXX_BASE_ADDR    0x60    /*!< base slave address for MCP47FXBXX */
    /* for volatile device MCP47FVBXX the i2c slave address is fixed
     * for nonvolatile device the base-address can be modified
     * this function is not implemented in this driver 
     */

// registers volatile
#define MCP47FXBXX_DAC0_vol     0x00
#define MCP47FXBXX_DAC1_vol     0x01
#define MCP47FXBXX_DAC2_vol     0x02
#define MCP47FXBXX_DAC3_vol     0x03
#define MCP47FXBXX_DAC4_vol     0x04
#define MCP47FXBXX_DAC5_vol     0x05
#define MCP47FXBXX_DAC6_vol     0x06
#define MCP47FXBXX_DAC7_vol     0x07
#define MCP47FXBXX_VREF_vol     0x08
#define MCP47FXBXX_PWR_DOWN_vol 0x09
#define MCP47FXBXX_STATUS_vol   0x0A
#define MCP47FXBXX_GAIN_vol     0x0A
#define MCP47FXBXX_WIPERLOCK_STATUS  0x0B

// registers non-volatile
#define MCP47FXBXX_DAC0_nvm     0x10
#define MCP47FXBXX_DAC1_nvm     0x11
#define MCP47FXBXX_DAC2_nvm     0x12
#define MCP47FXBXX_DAC3_nvm     0x13
#define MCP47FXBXX_DAC4_nvm     0x14
#define MCP47FXBXX_DAC5_nvm     0x15
#define MCP47FXBXX_DAC6_nvm     0x16
#define MCP47FXBXX_DAC7_nvm     0x17
#define MCP47FXBXX_VREF_nvm     0x18
#define MCP47FXBXX_PWR_DOWN_nvm 0x19
#define MCP47FXBXX_I2C_ADDR_cfg 0x1A
#define MCP47FXBXX_GAIN_nvm     0x1A


// commands paired with addresses (already add 0 on LSB)
#define MCP47FXBXX_WRITE        0x00
#define MCP47FXBXX_READ         0x06
#define MCP47FXBXX_CFG_EN       0x04
#define MCP47FXBXX_CFG_DIS      0x02

// General Call Commands - are sent to address 00h
// --> All devices can receive them simultaniously
#define MCP47FXBXX_RESET        0x06
#define MCP47FXBXX_WAKE_UP      0x0A

// control macros
    //Voltage reference control register
#define VREF_BUFFERED           0x3
#define VREF_UNBUFFERED         0x2
#define BANDGAP                 0x1 //vref voltage driven when powered down
#define VDD_UNBUFFERED          0x0
    //power down control register
#define POWER_DOWN_HIGH_Z       0x3
#define POWER_DOWN_125k         0x2
#define POWER_DOWN_1k           0x1
#define NORMAL_OPERATION        0x0
    // gain control
#define GAIN_X1                 0x0
#define GAIN_X2                 0x1
    
    // lock control
    // lock/unlock  DL->nonvolatile; CL->volatile;   DLn:CLn
    // HVC Pin must be greater than ~9V
#define FULL_LOCK               0x3 //neither DAC registers or DAC config can be accessed
#define VOLATILE_CFG_UNLOCKED   0x2 //volatile DAC config registers can be accessed
#define VOLATILE_UNLOCK         0x1 //volatile DAC registers and config can be accessed
#define FULL_UNLOCK             0x0 //volatile and non-volatile DAC registers and config can be accessed

class MCP47FXBXX {
public:
    uint16_t dac_data_public[8];

    /**
     * @brief Construct a new MCP47FXBXX object
     * @param [in] i2c driver instance
     * @param [in] AD0 logic state of the AD0-Pin (High=1; Low=0)
     * @param [in] AD1 logic state of the AD1-Pin (High=1; Low=0)
     * @return N/A
     */
    MCP47FXBXX(I2CArbiter& i2c, bool AD0, bool AD1);

    /**
     * @brief Destroy the MCP47FXBXX object
     * @return N/A
     */
    ~MCP47FXBXX();

    /**
     * @brief General Call Command to all I2C participants to issue a reset of the device
     *  specified by the I2C spec
     * @return Error case
     */
    esp_err_t reset(void);

    /**
     * @brief General Call Command to all I2C participants to wake up
     *  (exiting the power down mode)
     * @return Error case
     */
    esp_err_t wakeup(void);

    /**
     * @brief write data to DAC channel in specified memory
     * @param [in] channel: DAC channel 0-7
     * @param [in] value: DAC value 16bit
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case
     */
    esp_err_t writeDacValue(uint8_t channel, uint16_t value, bool mem_type);

    /**
     * @brief write data to all dac channels in volatile memory
     *  store data to be written in dac_data_public[]; [0] = channel 0
     *  no speed difference to single write noticed
     * @return Error case
     */
    esp_err_t writeDacValueAll(void);

    /**
     * @brief read the stored value of one dac channel from the specified memory
     * @param [in] channel: dac channel 0-7
     * @param [out] value: variable which stores read data
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case
     */
    esp_err_t readDacValue(uint8_t channel, uint16_t *value, bool mem_type);

    /**
     * @brief read stored dac values in specified register
     *  read dac_data_public[] for results; [0] = channel 0
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case
     */
    esp_err_t readDacValueAll(bool mem_type);

    /**
     * @brief set the power configuration for all channels
     * @param [in] vref_mode: value for voltage reference control register
     * @param [in] pwr_down_mode: value for power down control register
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case.
     */
    esp_err_t setPowerConfig(uint16_t vref_mode, uint16_t pwr_down_mode, bool mem_type);

    /**
     * @brief get the current power configuration for all channels
     * @param [out] vref_mode: variable for the voltage reference control register
     * @param [out] pwr_down_mode: variable for the power down control register
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case
     */
    esp_err_t getPowerConfig(uint16_t *vref_mode, uint16_t *pwr_down_mode, bool mem_type);

    /**
     * @brief check if the EEPROM write cycle is occuring
     * @param [out] state: variable to store returning data
     *  1 if EEPROM write is occuring, 0 if not
     * @return Error case
     */
    esp_err_t getEewaFlag(bool *state);

    /**
     * @brief check if power on reset or brown out reset has occured since the last read of this flag
     * @param [out] state: variable to store returning data
     *  1 if POR/BOR event has occured since the last read of this register
     * @return Error case
     */
    esp_err_t getPorFlag(bool *state);

    /**
     * @brief set the gain for the channels in the specified memory
     * @param [in] value: 8 bit write data
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case.
     */
    esp_err_t setGain(uint8_t value, bool mem_type);

    /**
     * @brief Get the gain setting for the specified memory
     * @param [out] value: variabel for the return value
     * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
     * @return Error case.
     */
    esp_err_t getGain(uint8_t *value, bool mem_type);

    /**
     * @brief Get the lock status word
     * @param [out] value containing the wiperlock technology status register
     * @return Error case.
     */
    esp_err_t getLockStatus(uint16_t *value);

    /**
     * @brief set lock/unlock state for specific channel
     *  HVC pin must be greater than ~9V for this to be effective
     * @param [in] channel: set bit for selected channel
     *  multiple channels can be selected (LSB = CH0)
     * @param [in] lock_state: enter desired lock state
     *  FULL_LOCK; VOLATILE_CFG_UNLOCKED; VOLATILE_UNLOCKED; FULL_UNLOCK
     * @return Error case
     */
    esp_err_t registerLock(uint8_t channel, uint8_t lock_state);

private:
    I2CArbiter&     m_i2c;          /*!< extern i2c instance */
    
    uint8_t         m_i2cAddress;   /*!< i2c Address including data from aADR-Pins */

    /**
     * @brief helper function to generate 2 bytes from selected channel and mode
     * @param [in] channel: selected channel
     * @param [in] lock_state: lock state as in .hpp
     * @return uint16_t 2 byte commands [15:8 nvm dac; 7:0 volatile dac]
     */
    uint16_t craftLockCommands(uint8_t channel, uint8_t lock_state);

    /**
     * @brief write command
     * @param [in] cmd: enter 8-bit command
     *  MCP47FXBXX_RESET
     *  MCP47FXBXX_WAKE_UP
     *  REG + MCP47FXBXX_CFG_EN
     *  REG + MCP47FXBXX_CFG_DIS
     *  reg + cmd manupulation must be performed befor calling this function
     * @return Error case
     */
    esp_err_t command(uint8_t cmd);

    /**
     * @brief write a single word to an specified register
     *  volatile and non-volatile memory
     * @param [in] reg: register to write to
     * @param [in] dat: data word to be written
     * @return Error case.
     */
    esp_err_t write(uint8_t reg, uint16_t dat);

    /**
     * @brief continously write 8 words to Device (volatile memory only)
     *  starting with at DAC volatile 0
     * @return Error case.
     */
    esp_err_t writeContinously();

    /**
     * @brief read a single word from a specified register
     * @param [in] reg: register to read from
     * @param [out] dat: variable to store read data to
     * @return Error case.
     */
    esp_err_t read(uint8_t reg, uint16_t *dat);
};
#endif /* MAIN_DRIVER_MCP47FXBXX_H */