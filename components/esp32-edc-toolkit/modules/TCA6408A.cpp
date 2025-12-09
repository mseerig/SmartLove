/**
 * @file TCA6408A.cpp
 * @author Erik Friedel (erik.friedel@ed-chemnitz.de)
 * @brief Low-Voltage 8-Bit I2C and SMBus I/O Expander with interrupt Output, Reset, and Configuration Registers
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "TCA6408A.hpp"
#include "System.hpp"

static const char* LOGTAG = "TCA6408A";

#include "sdkconfig.h"

/**
 * @brief Construct an instance of the class.
 * @param [in] i2c driver instance
 * @return N/A
 */
TCA6408A::TCA6408A(I2CArbiter& i2c, bool ADDR_PIN):
m_i2c(i2c) {
    TCA6408A::m_i2cAddress = TCA6408A_BASE_ADDR;
    if(ADDR_PIN)
        TCA6408A::m_i2cAddress |= TCA6408A_ADDR_OFFSET;
    ESP_LOGI(LOGTAG, "I2C ADDRESS: 0x%2x",TCA6408A::m_i2cAddress);
}

/**
 * @brief Class instance destructor.
 * @param N/A
 * @return N/A
 */
TCA6408A::~TCA6408A(){
    // I2C Stop!
}

/**
 * @brief Initial command for checking if the expander is alive
 * @param N/A
 * @return
 * -    ESP_OK -> Expander is alive.
 * -    ESP_FAIL -> Expander has a failure.
 */
esp_err_t TCA6408A::init(void) {
    esp_err_t ret = ESP_FAIL;
    uint8_t _data;
    uint8_t _pol;
    
    ret = TCA6408A::setupPortRead(&_data, &_pol);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"init failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief Configure Pin direction (I/O) and read polarity of a pin
 * @param [in] pin (0-7)
 * @param [in] direction: 0=Output, 1=Input
 * @param [in] pol: 0=Read data is not inverted, 1=Read data is inverted
 * @return Error case.
 */
esp_err_t TCA6408A::setupPinWrite(uint8_t pin, uint8_t direction, uint8_t pol){
    esp_err_t ret = ESP_FAIL;
    uint8_t _old_direction = 0;
    uint8_t _old_pol = 0;

    if(pin > 7 || direction > 1 ||pol > 1){
        ret = ESP_ERR_INVALID_ARG;
        ESP_LOGI(LOGTAG,"invalid argument - Errorcode: %d",ret);
        return ret;
    }

    //read exisiting register
    ret = TCA6408A::readRegister(TCA6408A_CONFIG,&_old_direction);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    ret = TCA6408A::readRegister(TCA6408A_POLARITY,&_old_pol);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }

    //modify exisiting data
    if(direction == 0)
        direction = _old_direction & ~(1<<pin);
    else
        direction = _old_direction | (1<<pin);
    
    if(pol == 0)
        pol = _old_pol & ~(1<<pin);
    else
        pol = _old_pol | (1<<pin);

    //write new data into registers
    ret = TCA6408A::writeRegister(TCA6408A_CONFIG, direction);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    ret = TCA6408A::writeRegister(TCA6408A_POLARITY, pol);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief Configure direction (I/O) and read polarity; LSB=Pin0
 * @param [in] direction: 0=Output, 1=Input
 * @param [in] pol: 0=Read data is not inverted, 1=Read data is inverted
 * @return Error case.
 */
esp_err_t TCA6408A::setupPortWrite(uint8_t direction, uint8_t pol){
    esp_err_t ret = ESP_FAIL;

    ret = TCA6408A::writeRegister(TCA6408A_CONFIG,direction);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write port setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }

    ret = TCA6408A::writeRegister(TCA6408A_POLARITY,pol);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write port setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief Read Configuration of Pin direction (I/O) and read polarity
 * @param [in] pin (0-7)
 * @param [out] direction: 0=Output, 1=Input
 * @param [out] pol: 0=Read data is not inverted, 1=Read data is inverted
 * @return Error case.
 */
esp_err_t TCA6408A::setupPinRead(uint8_t pin, uint8_t *direction, uint8_t *pol){
    esp_err_t ret = ESP_FAIL;

    uint8_t _direction = 0;
    uint8_t _polarity = 0;

    if(pin > 7){
        ret = ESP_ERR_INVALID_ARG;
        ESP_LOGI(LOGTAG,"invalid argument - Errorcode: %d",ret);
        return ret;
    }

    ret = TCA6408A::readRegister(TCA6408A_CONFIG,&_direction);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"read pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }

    ret = TCA6408A::readRegister(TCA6408A_POLARITY,&_polarity);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"read pin setup failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }

    *direction = (_direction >> pin) & 0x1;
    *pol = (_polarity >> pin) & 0x1;

    return ESP_OK;
}

/**
 * @brief Read Configuration of port direction (I/O) and read polarity; LSB=Pin0
 * @param [out] direction: 0=Output, 1=Input
 * @param [out] pol: 0=Read data is not inverted, 1=Read data is inverted
 * @return Error case.
 */
esp_err_t TCA6408A::setupPortRead(uint8_t *direction, uint8_t *pol){
    esp_err_t ret = ESP_FAIL;

    ret = TCA6408A::readRegister(TCA6408A_CONFIG,direction);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"read port setup failed - Errorcode: %d",ret);
        return ret;
    }

    ret = TCA6408A::readRegister(TCA6408A_POLARITY,pol);
    if(ret != ESP_OK)
        ESP_LOGI(LOGTAG,"read port setup failed - Errorcode: %d",ret);
    return ret;
}

/**
 * @brief Set state for output pin (has no effect on Input pins)
 * @param [in] pin: (0-7)
 * @param [in] state: True = Set, False = Reset
 * @return Error Case.
 */
esp_err_t TCA6408A::pinWrite(uint8_t pin, uint8_t state){
    esp_err_t ret = ESP_FAIL;
    uint8_t _oldstate = 0;

    if(pin > 7 || state > 1){
        ret = ESP_ERR_INVALID_ARG;
        return ret;
    }

    ret = TCA6408A::readRegister(TCA6408A_OUTPUT,&_oldstate);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write pin failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }

    if(state == 0)
        state = _oldstate & ~(1<<pin);
    else
        state = _oldstate | (1<<pin);

    ret = TCA6408A::writeRegister(TCA6408A_OUTPUT, state);
    if(ret != ESP_OK)
        ESP_LOGI(LOGTAG,"write pin failed - Errorcode: %d",ret);
    return ret;
}

/**
 * @brief Set state for output port (has no effect on Input pins); LSB=Pin0
 * @param [in] state: True = Set, False = Reset
 * @return Error Case.
 */
esp_err_t TCA6408A::portWrite(uint8_t state){
    esp_err_t ret = ESP_FAIL;
    ret = TCA6408A::writeRegister(TCA6408A_OUTPUT,state);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"write port failed - Errorcode: %d",ret);
    }
    return ret;
}

/**
 * @brief Read state of input pin (also returns the set value for output pins)
 *  writing 1 to the corresponding polarity inversion register inverts the state.
 * @param [in] pin: (0-7)
 * @param [out] state: True = Set, False = Reset (if polarity inversion is not set)
 * @return Error Case.
 */
esp_err_t TCA6408A::pinRead(uint8_t pin, uint8_t *state){
    esp_err_t ret = ESP_FAIL;
    uint8_t _data;

    if(pin > 7){
        ret = ESP_ERR_INVALID_ARG;
        ESP_LOGI(LOGTAG,"invalid argument - Errorcode: %d",ret);
        return ret;
    }

    ret = TCA6408A::readRegister(TCA6408A_INPUT,&_data);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"read pin failed - Errorcode: %d",ret);
        return ESP_FAIL;
    }
    
    *state = (_data >> pin) & 0x1;
    return ESP_OK;
}

/**
 * @brief Read state of input pin (also returns the set value for output pins), LSB=Pin0
 *  writing 1 to the corresponding polarity inversion register inverts the state.
 * @param [out] state: True = Set, False = Reset (if polarity inversion is not set)
 * @return Error Case.
 */
esp_err_t TCA6408A::portRead(uint8_t *state){
    esp_err_t ret = ESP_FAIL;

    ret = TCA6408A::readRegister(TCA6408A_INPUT,state);
    if(ret!=ESP_OK)
        ESP_LOGI(LOGTAG,"read port failed - Errorcode: %d",ret);
    return ret;
}

/**
 * @brief wirte a single register to the TCA6408A
 * @param [in] reg: register address
 * @param [in] dat: data to write in the register
 * @return Error case.
 */
esp_err_t TCA6408A::writeRegister(uint8_t reg, uint8_t dat){
    esp_err_t ret = ESP_FAIL;

    uint8_t buff[2]  ={reg, dat};
    ret = m_i2c.acquireBus();

    if(ret == ESP_OK) m_i2c.setAddress(TCA6408A::m_i2cAddress);
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(buff, 2, I2C_MASTER_ACK);

    //end Transaction anyway!
    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ESP_OK;
    }
    m_i2c.releaseBus();   //release bus anyway
    return ESP_FAIL;
}

/**
 * @brief read a single register from the TCA6408A
 * @param [in] reg: register address
 * @param [out] dat: data variable for response
 * @return Error case.
 */
esp_err_t TCA6408A::readRegister(uint8_t reg, uint8_t *dat){
    esp_err_t ret = ESP_FAIL;

    ret = m_i2c.acquireBus();
    if(ret == ESP_OK) m_i2c.setAddress(TCA6408A::m_i2cAddress);
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(reg, I2C_MASTER_ACK);
    if(ret == ESP_OK) ret = m_i2c.endTransaction();
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.read(dat, I2C_MASTER_NACK);

    //end transaction anyway!
    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ESP_OK;
    }
    m_i2c.releaseBus();   //release Bus anyway
    return ESP_FAIL;
}