/**
 * @file MCP47FXBXX.cpp
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

#include "MCP47FXBXX.hpp"
#include "System.hpp"

static const char* LOGTAG = "MCP47FXBXX";

#include "sdkconfig.h"

/**
 * @brief Construct a new MCP47FXBXX object
 * @param [in] i2c driver instance
 * @param [in] AD0 logic state of the AD0-Pin (High=1; Low=0)
 * @param [in] AD1 logic state of the AD1-Pin (High=1; Low=0)
 * @return N/A
 */
MCP47FXBXX::MCP47FXBXX(I2CArbiter& i2c, bool AD0, bool AD1):
        m_i2c(i2c) {
    MCP47FXBXX::m_i2cAddress = MCP47FXBXX_BASE_ADDR;
    if(AD0)
        MCP47FXBXX::m_i2cAddress |= MCP47FXBXX_AD0_OFFSET;
    if(AD1)
        MCP47FXBXX::m_i2cAddress |= MCP47FXBXX_AD1_OFFSET;
    for(uint8_t i=0; i<8; i++){
        dac_data_public[i]=0;
    }
}

/**
 * @brief Destroy the MCP47FXBXX object
 * @return N/A
 */
MCP47FXBXX::~MCP47FXBXX(){
    // I2C Stop!
}

/**
 * @brief General Call Command to all I2C participants to issue a reset of the device
 *  specified by the I2C spec
 * @return Error case
 */
esp_err_t MCP47FXBXX::reset(void){
    esp_err_t ret = ESP_FAIL;

    ret = MCP47FXBXX::command(MCP47FXBXX_RESET);

    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief General Call Command to all I2C participants to wake up
 *  (exiting the power down mode)
 * @return Error case
 */
esp_err_t MCP47FXBXX::wakeup(void){
    esp_err_t ret = ESP_FAIL;

    ret = MCP47FXBXX::command(MCP47FXBXX_WAKE_UP);

    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief write data to DAC channel in specified memory
 * @param [in] channel: DAC channel 0-7
 * @param [in] value: DAC value 16bit
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case
 */
esp_err_t MCP47FXBXX::writeDacValue(uint8_t channel, uint16_t value, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint8_t reg = 0;

    if(channel>7)
        return ESP_ERR_INVALID_ARG;

    if(mem_type == true){
         reg = MCP47FXBXX_DAC0_vol + channel;
    }
    else{
        reg = MCP47FXBXX_DAC0_nvm + channel;
    }

    ret = MCP47FXBXX::write(reg, value);
    

    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief write data to all dac channels in volatile memory
 *  store data to be written in dac_data_public[]; [0] = channel 0
 *  no speed difference to single write noticed
 * @return Error case
 */
esp_err_t MCP47FXBXX::writeDacValueAll(void){
    esp_err_t ret = ESP_FAIL;

    ret = writeContinously();

    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief read the stored value of one dac channel from the specified memory
 * @param [in] channel: dac channel 0-7
 * @param [out] value: variable which stores read data
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case
 */
esp_err_t MCP47FXBXX::readDacValue(uint8_t channel, uint16_t *value, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    uint8_t reg = 0;
    if(channel>7)
        return ESP_ERR_INVALID_ARG;

    if(mem_type == true){
         reg = MCP47FXBXX_DAC0_vol + channel;
    }
    else{
        reg = MCP47FXBXX_DAC0_nvm + channel;
    }

    ret = MCP47FXBXX::read(reg,&_buff);
    if(ret != ESP_OK)
        return ESP_FAIL;
    *value = _buff;
    return ESP_OK;
}

/**
 * @brief read stored dac values in specified register
 *  read dac_data_public[] for results
 * @param [in] mem_type:  choose memory: 0 = non volatile; 1 = volatile
 * @return Error case
 */
esp_err_t MCP47FXBXX::readDacValueAll(bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint8_t reg = 0;
    uint16_t data=0;
    if(mem_type == true){
        reg = MCP47FXBXX_DAC0_vol;
    }
    else{
        reg = MCP47FXBXX_DAC0_nvm;
    }

    for(uint8_t i=0;i<8;i++){
        MCP47FXBXX::read(reg+i,&data);
        dac_data_public[i] = data;
    }

    if(ret != ESP_OK){
        return ESP_FAIL;
    }
    
    // for(uint8_t i=0; i<8; i++){
    //     MCP47FXBXX::dac_data_public[i] = MCP47FXBXX::_dac_data[i];
    // }
    ESP_LOGI(LOGTAG,"/d data transfer finished");
    return ESP_OK;
}

/**
 * @brief set the power configuration for all channels
 * @param [in] vref_mode: value for voltage reference control register
 * @param [in] pwr_down_mode: value for power down control register
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case.
 */
esp_err_t MCP47FXBXX::setPowerConfig(uint16_t vref_mode, uint16_t pwr_down_mode, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    
    if(mem_type == true){       //choose memory to write to
        ret = MCP47FXBXX::write(MCP47FXBXX_VREF_vol,vref_mode);
        if(ret != ESP_OK)
            return ESP_FAIL;
        ret = MCP47FXBXX::write(MCP47FXBXX_PWR_DOWN_vol,pwr_down_mode);
    }
    else{
        ret = MCP47FXBXX::write(MCP47FXBXX_VREF_nvm,vref_mode);
        if(ret != ESP_OK)
            return ESP_FAIL;
        ret = MCP47FXBXX::write(MCP47FXBXX_PWR_DOWN_nvm,pwr_down_mode);
    }

    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief get the current power configuration for all channels
 * @param [out] vref_mode: variable for the voltage reference control register
 * @param [out] pwr_down_mode: variable for the power down control register
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case
 */
esp_err_t MCP47FXBXX::getPowerConfig(uint16_t *vref_mode, uint16_t *pwr_down_mode, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint16_t _vref, _pwr_down = 0;

    if(mem_type == true){       //choose memory to read from
        ret = MCP47FXBXX::read(MCP47FXBXX_VREF_vol,&_vref);
        if(ret!=ESP_OK)
            return ESP_FAIL;
        ret = MCP47FXBXX::read(MCP47FXBXX_PWR_DOWN_vol,&_pwr_down);
    }
    else{
        ret = MCP47FXBXX::read(MCP47FXBXX_VREF_nvm,&_vref);
        if(ret!=ESP_OK)
            return ESP_FAIL;
        ret = MCP47FXBXX::read(MCP47FXBXX_PWR_DOWN_nvm,&_pwr_down);
    }
    if(ret!=ESP_OK)
        return ESP_FAIL;

    *vref_mode = _vref;
    *pwr_down_mode = _pwr_down;
    return ESP_OK;
}

/**
 * @brief check if the EEPROM write cycle is occuring
 * @param [out] state: variable to store returning data
 *  1 if EEPROM write is occuring, 0 if not
 * @return Error case
 */
esp_err_t MCP47FXBXX::getEewaFlag(bool *state){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    ret = MCP47FXBXX::read(MCP47FXBXX_STATUS_vol,&_buff);
    if(ret == ESP_OK){
        *state = (bool)((_buff >> 6) & 0x1); //shift bit in question to LSB and mask out the rest
        return ESP_OK;
    }
    return ESP_FAIL;
}

/**
 * @brief check if power on reset or brown out reset has occured since the last read of this flag
 * @param [out] state: variable to store returning data
 *  1 if POR/BOR event has occured since the last read of this register
 * @return Error case
 */
esp_err_t MCP47FXBXX::getPorFlag(bool *state){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    ret = MCP47FXBXX::read(MCP47FXBXX_STATUS_vol,&_buff);
    if(ret == ESP_OK){
        *state = (bool)((_buff >> 7) & 0x1); //shift bit in question to LSB and mask out the rest
        return ESP_OK;
    }
    return ESP_FAIL;
}

/**
 * @brief set the gain for the channels in the specified memory
 * @param [in] value: 8 bit write data
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case.
 */
esp_err_t MCP47FXBXX::setGain(uint8_t value, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    
    // get current gain status - register also contain other information
    // which must be preserved
    if(mem_type == true)
        ret = MCP47FXBXX::read(MCP47FXBXX_GAIN_vol,&_buff);
    else
        ret = MCP47FXBXX::read(MCP47FXBXX_GAIN_nvm,&_buff);
    if(ret != ESP_OK)
        return ESP_FAIL;
    
    _buff = _buff & 0x00FF;     //clear gain bits
    _buff |= ((uint16_t) value )<<8;          //add new gain bits

    if(mem_type == true)
        ret = MCP47FXBXX::write(MCP47FXBXX_GAIN_vol,_buff);
    else
        ret = MCP47FXBXX::write(MCP47FXBXX_GAIN_nvm,_buff);
    if(ret != ESP_OK)
        return ESP_FAIL;
    return ESP_OK;
}

/**
 * @brief Get the gain setting for the specified memory
 * @param [out] value: variabel for the return value
 * @param [in] mem_type: choose memory: 0 = non volatile; 1 = volatile
 * @return Error case.
 */
esp_err_t MCP47FXBXX::getGain(uint8_t *value, bool mem_type){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    if(mem_type == true)
        ret = MCP47FXBXX::read(MCP47FXBXX_GAIN_vol,&_buff);
    else
        ret = MCP47FXBXX::read(MCP47FXBXX_GAIN_nvm,&_buff);
    if(ret != ESP_OK)
        return ESP_FAIL;
    
    *value = (uint8_t)(_buff >> 8);
    return ESP_OK;
}

/**
 * @brief Get the lock status word
 * @param [out] value containing the wiperlock technology status register
 * @return Error case.
 */
esp_err_t MCP47FXBXX::getLockStatus(uint16_t *value){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buff=0;
    ret = MCP47FXBXX::read(MCP47FXBXX_WIPERLOCK_STATUS,&_buff);
    if(ret == ESP_OK){
        *value = _buff;
        return ESP_OK;
    }
    return ESP_FAIL;
}

/**
 * @brief set lock/unlock state for specific channel
 *  HVC pin must be greater than ~9V for this to be effective
 * @param [in] channel: set bit for selected channel
 *  multiple channels can be selected (LSB = CH0)
 * @param [in] lock_state: enter desired lock state
 *  FULL_LOCK; VOLATILE_CFG_UNLOCKED; VOLATILE_UNLOCKED; FULL_UNLOCK
 * @return Error case
 */
esp_err_t MCP47FXBXX::registerLock(uint8_t channel, uint8_t lock_state){
    esp_err_t ret = ESP_FAIL;
    uint16_t _buffff = 0;

    for(uint8_t i=0;i<8;i++){
        if((channel & 1<<i) == true){
            _buffff = craftLockCommands(channel,lock_state);
            ret = MCP47FXBXX::command((uint8_t)_buffff >> 8);
            if(ret==ESP_OK) ret = MCP47FXBXX::command((uint8_t)(_buffff & 0xff));
        }
        if(ret!=ESP_OK)
            return ESP_FAIL;
    }
    return ESP_OK;
}

/* ########## private functions ########## */

/**
 * @brief helper function to generate 2 bytes from selected channel and mode
 * @param [in] channel: selected channel
 * @param [in] lock_state: lock state as in .hpp
 * @return uint16_t 2 byte commands [15:8 nvm dac; 7:0 volatile dac]
 * disable bit to unlock
 */
uint16_t craft_lock_commands(uint8_t channel, uint8_t lock_state){
    uint16_t _command=0;
    if(channel>=8)
        return 0;

    //apply DLn and CLn bits
    switch(lock_state){
        case FULL_LOCK:{
            _command |= (MCP47FXBXX_CFG_EN <<8 | MCP47FXBXX_CFG_EN); break;
            }
        case VOLATILE_CFG_UNLOCKED:{
            _command |= (MCP47FXBXX_CFG_EN <<8 | MCP47FXBXX_CFG_DIS); break;
            } 
        case VOLATILE_UNLOCK:{
            _command |= (MCP47FXBXX_CFG_DIS <<8 | MCP47FXBXX_CFG_EN); break;
        }
        case FULL_UNLOCK:{
            _command |= (MCP47FXBXX_CFG_DIS <<8 | MCP47FXBXX_CFG_DIS); break;
        }
        default: return ESP_ERR_INVALID_ARG;
    }

    //automated register selector - see .hpp for absolut registers
    //non-volatile register (channel | 0x10)
    //volatile register (channel | 0x0) -> only channel
    _command |= (((channel | 0x10)<<8)<<3) | (channel <<3);

    // command = 0b R00CCxxz R00CCyyz
    // R = 1 for non volatile; 0 for volatile
    // CC = channel (0-7)
    // xx = enable/disable command for non volatile memory
    // yy = enable/disable command for volatile memory
    // z = reserved, set to 0
    
    return _command;
}

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
esp_err_t MCP47FXBXX::command(uint8_t cmd){
    esp_err_t ret = ESP_FAIL;

    ret = m_i2c.acquireBus();
    if(ret==ESP_OK){
        if(cmd == MCP47FXBXX_WAKE_UP || cmd == MCP47FXBXX_RESET)
            m_i2c.setAddress(0);  //general command dont need address
        else
            m_i2c.setAddress(MCP47FXBXX::m_i2cAddress);
    } 
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(cmd, I2C_MASTER_ACK);

    //end Transaction anyway!
    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ESP_OK;
    }
    m_i2c.releaseBus();   //release bus anyway
    return ESP_FAIL;
}

/**
 * @brief write a single word to an specified register
 *  volatile and non-volatile memory
 * @param [in] reg: register to write to
 * @param [in] dat: data word to be written
 * @return Error case.
 */
esp_err_t MCP47FXBXX::write(uint8_t reg, uint16_t dat){
    esp_err_t ret = ESP_FAIL;
    uint8_t buff[3] = {0};
    
    buff[0] = (reg <<3) | MCP47FXBXX_WRITE;    //add command bits
    buff[1] = dat >> 8;
    buff[2] = (uint8_t) dat & 0xff;

    ret = m_i2c.acquireBus();

    if(ret == ESP_OK) m_i2c.setAddress(MCP47FXBXX::m_i2cAddress);
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(buff, 3, I2C_MASTER_ACK);

    //end Transaction anyway!
    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ESP_OK;
    }
    m_i2c.releaseBus();   //release bus anyway
    ESP_LOGI(LOGTAG,"write failed");
    return ESP_FAIL;
}

/**
 * @brief continously write 8 words to Device (volatile memory only)
 *  starting with at DAC volatile 0
 * @return Error case.
 */
esp_err_t MCP47FXBXX::writeContinously(void){
    esp_err_t ret = ESP_FAIL;
    uint8_t reg = MCP47FXBXX_DAC0_vol;
    uint8_t buff[24] = {0}; //8x 1address byte + 2 data bytes

    for(uint8_t i=0;i<8;i++){
        buff[(i*3)+0] = ((reg+i) <<3) | MCP47FXBXX_WRITE;    //add command bits
        buff[(i*3)+1] = dac_data_public[i] >> 8;
        buff[(i*3)+2] = (uint8_t) dac_data_public[i] & 0xff;
    }
    
    ret = m_i2c.acquireBus();

    if(ret == ESP_OK) m_i2c.setAddress(MCP47FXBXX::m_i2cAddress);
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(buff,24,I2C_MASTER_ACK);

    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ESP_OK;
    }
    ESP_LOGI(LOGTAG,"write continously failed");
    return ESP_FAIL;
}

/**
 * @brief read a single word from a specified register
 * @param [in] reg: register to read from
 * @param [out] dat: variable to store read data to
 * @return Error case.
 */
esp_err_t MCP47FXBXX::read(uint8_t reg, uint16_t *dat){
    esp_err_t ret = ESP_FAIL;
    uint8_t data[2] ={0};
    reg = reg <<3 | MCP47FXBXX_READ;    //add command bits


    ret = m_i2c.acquireBus();
	if (ret == ESP_OK) m_i2c.setAddress(m_i2cAddress);
	if (ret == ESP_OK) ret = m_i2c.beginTransaction();
	if (ret == ESP_OK) ret = m_i2c.write(reg, I2C_MASTER_ACK);
	if (ret == ESP_OK) ret = m_i2c.endTransaction();
	if (ret == ESP_OK) ret = m_i2c.beginTransaction();
	if (ret == ESP_OK) ret = m_i2c.read(data, 2, I2C_MASTER_LAST_NACK);

	// endTransaction anyway!
 	if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
		m_i2c.releaseBus();
        *dat = data[1]<<8|data[0];
		return ESP_OK;
	}
	m_i2c.releaseBus();	//release bus anyway
	return ESP_FAIL;
}