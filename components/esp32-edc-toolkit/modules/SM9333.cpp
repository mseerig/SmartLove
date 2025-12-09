/**
 * @file SM9333.hpp
 * @author Erik Friedel (erik.friedel@ed-chemnitz.de)
 * @brief Digital Pressure and Altimeter Sensor Module
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "SM9333.hpp"
#include "System.hpp"

static const char* LOGTAG = "SM9333";

#include "sdkconfig.h"

/**
 * @brief Construct an instance of the class.
 * @param [in] i2c driver instance
 * @return N/A
 */
SM9333::SM9333(I2CArbiter& i2c):
m_i2c(i2c){

}

/**
 * @brief Class instance destructor
 * @param N/A
 * @return N/A
 */
SM9333::~SM9333(){
    // I2C Stop!
}

/**
 * @brief Initial command for checking if the device is measuring values
 *  reads serial number and stores them in private variable
 * @param N/A
 * @return
 * -    ESP_OK -> Sensor is alive and ready
 * -    ESP_FAIL -> Sensor is not ready or not working
 */
esp_err_t SM9333::init(void){
    int ret = ESP_FAIL;
    uint16_t _data;

    ret = SM9333::readRegister(SM9333_STATUS_REG_ADDR, &_data);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"Reg read failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    m_status = _data;

    if(m_status & 0x1)     //check if sensor is busy
        ESP_LOGI(LOGTAG,"Sensor idle");
    else{
        ESP_LOGI(LOGTAG,"Sensor busy");
    }
    
    if((m_status&0x18) == 0x18){   //check if bit 3 and 4 are high - sensor is ready
        m_temp_ready = true;
        m_press_ready = true;
        ret = ESP_OK;
        ESP_LOGI(LOGTAG,"Sensor ready");
   }
    else{
        m_temp_ready = false;
        m_press_ready = false;
        ESP_LOGI(LOGTAG,"Sensor not ready");
        return ESP_FAIL;
    }

    _data = 0;
    ret = SM9333::readRegister(SM9333_SERIAL_L_REG_ADDR, &_data);
    if(ret != ESP_OK) return ESP_FAIL;
    m_serial = _data;
    _data = 0;
    ret = SM9333::readRegister(SM9333_SERIAL_H_REG_ADDR, &_data);
    if(ret != ESP_OK) return ESP_FAIL;
    m_serial |= _data<<16;
    return ESP_OK;
}

/**
 * @brief Command to enter sleep mode on SM9333
 * @param N/A
 * @return Error case
 */
esp_err_t SM9333::sleepMode(void){
    int ret = ESP_FAIL;
    ret = SM9333::writeRegister(SM9333_COMMAND_REG_ADDR, SM9333_CMD_SLEEP);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"enter sleep mode failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    ESP_LOGI(LOGTAG,"enter sleep mode");
    return ESP_OK;
}

/**
 * @brief Command to wake SM9333 from sleep mode
 * @param N/A
 * @return Error case
 */
esp_err_t SM9333::wakeUp(void){
    int ret = ESP_FAIL;
    uint16_t _dump=0;
    ret = SM9333::readRegister(SM9333_STATUS_REG_ADDR, &_dump);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"wakeup failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    ESP_LOGI(LOGTAG,"wakeup");
    return ESP_OK;
    //TODO: check behaviour on wake up
    //is read value correct, which errors are normal, how to handle them?
}

/**
 * @brief Command to reset the sensor
 * @param N/A
 * @return Error case
 */
esp_err_t SM9333::reset(void){
    int ret = ESP_FAIL;
    ret = SM9333::writeRegister(SM9333_COMMAND_REG_ADDR, SM9333_CMD_RESET);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"reset command failed - Errorcode %d",ret);
    return ESP_FAIL;
    }
    ESP_LOGI(LOGTAG,"reset command issued");
    return ESP_OK;
}

/**
 * @brief gets serial number of sensor - if empty, run init first
 * @return Serial number
 */
uint32_t SM9333::getSerial(void){
    return m_serial;
}

/**
 * @brief gets the locally stored status value; use after measure_temperature() and measure_pressure()
 * @return last read status value
 */
uint16_t SM9333::getStatus(void){
    return m_status;
}

/**
 * @brief reads the status from the sensor
 * @param [out] dat: 16 bit data variable
 * @return Error case
 * 
 */
esp_err_t SM9333::readStatus(uint16_t *dat){
    int ret = ESP_FAIL;
    ret = SM9333::readRegister(SM9333_STATUS_REG_ADDR, dat);
    m_status = *dat;
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"read status failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief command to reset event flags in status
 * @param N/A
 * @return error case
 */
esp_err_t SM9333::resetStatusFlags(void){
    int ret = ESP_FAIL;
    ret = SM9333::writeRegister(SM9333_STATUS_REG_ADDR, 0xFFFF);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"enter sleep mode failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief get locally stored measured data for temperature and pressure; use after measure_all()
 * @param [out] temp: last valid temperature value
 * @param [out] pressure: last valid pressure value
 */
void SM9333::getMeasuredData(float *temp, float *pressure){
    *temp = 0;
    *pressure = 0;
    if(m_temp_ready)
        *temp = m_temperature;
    if(m_press_ready)
        *pressure = m_pressure;
}
    
/**
 * @brief get measured data from sensor and stores the valid values locally; use before get_measured_data()
 * @param N/A
 * @return Error case
 */
esp_err_t SM9333::measureAll(void){
    int ret = ESP_OK;
    float _data = 0;

    ret = SM9333::measureTemperature(&_data);
    if(ret == ESP_OK){
        m_temperature = _data;
    }
    else{
        ESP_LOGI(LOGTAG,"measure all command failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    _data = 0;

    ret = SM9333::measurePressure(&_data);
    if(ret == ESP_OK){
        m_pressure = _data;
        return ESP_OK;
    }
    ESP_LOGI(LOGTAG,"measure all command failed - Errorcode %d",ret);
    return ESP_FAIL;
}

/**
 * @brief measures the temperature and checks if value is valid
 *  call get_status() after this, if wanted
 * @param [out] temperature in °C
 * @return Error case
 */
esp_err_t SM9333::measureTemperature(float *temperature){
    int ret = ESP_FAIL;

    uint16_t _value;
    uint16_t _status;
    ret = SM9333::readRegister(SM9333_TEMP_REG_ADDR, &_value);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"temperature measurement failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    ret = SM9333::readRegister(SM9333_STATUS_SYNC_REG_ADDR, &_status);
    if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"temperature measurement failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    
    if(_status & 1<<4){
        m_temp_ready = true;
    }
    else{
        temperature = 0;
        m_temp_ready = false;
        ESP_LOGI(LOGTAG,"temperature measurement not ready - Errorcode %d",ret);
        return ESP_FAIL;
    }

    // temperature conversion

    int16_t integer = (int16_t)_value;
    m_temperature = (static_cast<float>(integer) - SM9333_TEMP_OFFSET ) / SM9333_TEMP_SENSITIVITY;
    // Data int16 = sensitivity * temperature in °C + offset
    // Data int16 = data uint16 - 32768
    // --> temperature in °C = ((uint16 -32768) - offset) / sensitivity
    //see datasheet chapter 12.2.4 Digital Temperature Transfer function
    *temperature = m_temperature;    //return value

    return ESP_OK;
}

/**
 * @brief measures the pressure and checks if value is valid
 *  call get_status() after this, if wanted
 * @param [out] pressure value in Pa
 * @return Error case
 */
esp_err_t SM9333::measurePressure(float *pressure){
    int ret = ESP_FAIL;    
    
    uint16_t _value;
    uint16_t _status;
    ret = SM9333::readRegister(SM9333_PRESSURE_REG_ADDR, &_value);
        if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"pressure measurement failed - Errorcode %d",ret);
        return ESP_FAIL;
    }
    ret = SM9333::readRegister(SM9333_STATUS_SYNC_REG_ADDR, &_status);
        if(ret != ESP_OK){
        ESP_LOGI(LOGTAG,"pressure measurement failed - Errorcode %d",ret);
        return ESP_FAIL;
    }

    if(_status & 1<<3){
        m_press_ready = true;
    }
    else{
        *pressure = 0;
        m_press_ready = false;
        ESP_LOGI(LOGTAG,"pressure measurement not ready - Errorcode %d",ret);
        return ESP_FAIL;
    }

    // pressure conversion
    int16_t integer = (int16_t)_value;
    m_pressure = (SM9333_PRESSURE_MIN + ((static_cast<float>(integer) + 26215)/52429.0f) * (SM9333_PRESSURE_MAX - SM9333_PRESSURE_MIN));
    // deteiled formula: p = pmin + ((data(int)-OUTmin) / (OUTmax - OUTmin) )(pmax - pmin);
    // pmin -125
    // pmax +125
    // OUT min -26215 counts
    // OUT max 26214 counts
    *pressure = m_pressure;         //return value

    return ESP_OK;
}

/**
 * @brief write a single 16-bit register to the SM9333
 * @param [in] reg: register address
 * @param [in] dat: data to write in the register
 * @return Error case
 */
esp_err_t SM9333::writeRegister(uint8_t reg, uint16_t dat){
    int ret = ESP_FAIL;

    uint8_t buff[3] = {reg, (uint8_t)(dat&0xff), (uint8_t)( (dat&0xff00) >>8 )};
    ret = m_i2c.acquireBus();

    if(ret == ESP_OK) m_i2c.setAddress(SM9333_I2C_BASE_ADDR);
    if(ret == ESP_OK) ret = m_i2c.beginTransaction();
    if(ret == ESP_OK) ret = m_i2c.write(buff, 3, I2C_MASTER_ACK);

    //end transaction anyway!
    if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
        m_i2c.releaseBus();
        return ret;
    }
    m_i2c.releaseBus();   //release bus anyway
    return ESP_FAIL;



}

/**
 * @brief read a single 16-bit register from SM9333
 * @param [in] reg: register address
 * @param [out] dat: data variable for response
 * @return Error case
 */
esp_err_t SM9333::readRegister(uint8_t reg, uint16_t *dat){
    int ret = ESP_FAIL;

    uint8_t bytes[2] = {0};

    ret = m_i2c.acquireBus();
	if (ret == ESP_OK) m_i2c.setAddress(SM9333_I2C_BASE_ADDR);
	if (ret == ESP_OK) ret = m_i2c.beginTransaction();
	if (ret == ESP_OK) ret = m_i2c.write(reg, I2C_MASTER_ACK);
	if (ret == ESP_OK) ret = m_i2c.endTransaction();
	if (ret == ESP_OK) ret = m_i2c.beginTransaction();
	if (ret == ESP_OK) ret = m_i2c.read(bytes, 2, I2C_MASTER_LAST_NACK);

	// endTransaction anyway!
 	if(m_i2c.endTransaction() == ESP_OK && ret == ESP_OK){
		m_i2c.releaseBus();
        *dat = bytes[1]<<8|bytes[0];
		return ESP_OK;
	}
	m_i2c.releaseBus();	//release bus anyway
	return ESP_FAIL;
}