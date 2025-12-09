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


#ifndef MAIN_DRIVERS_SM9333_H_
#define MAIN_DRIVERS_SM9333_H_

#include <stdio.h>      /* printf, scanf */
#include <string>
#include <sys/time.h>

#include <esp_log.h>
#include <esp_err.h>
#include "I2CArbiter.hpp"


//Address has only 7bit, because the "least significant bit" is reserved for ACK/NAK.
#define SM9333_I2C_BASE_ADDR     0x6C    /*!< slave address for SM9333 0x6C*/

#define SM9333_COMMAND_REG_ADDR      0x22    /*!< Command Register*/
#define SM9333_TEMP_REG_ADDR         0x2e    /*!< Temperature Register - corrected temperature measurement*/
#define SM9333_PRESSURE_REG_ADDR     0x30    /*!< Pressure Register - corrected pressure measurement*/
#define SM9333_STATUS_SYNC_REG_ADDR  0x32    /*!< mirrors the status register for continous read*/
#define SM9333_STATUS_REG_ADDR       0x36    /*!< status register*/
#define SM9333_SERIAL_L_REG_ADDR     0x50    /*!< Serial number lo-word*/
#define SM9333_SERIAL_H_REG_ADDR     0x52    /*!< Serial number hi-word*/

//commands
#define SM9333_CMD_SLEEP            0x6C32      /*!< Sleep command*/
#define SM9333_CMD_RESET            0xB169      /*!< Reset command*/

//transformation
#define SM9333_TEMP_OFFSET  -16881  // counts 
#define SM9333_TEMP_SENSITIVITY 397.2   // counts/°C

#define SM9333_PRESSURE_MIN -125
#define SM9333_PRESSURE_MAX 125


class SM9333 {
public:
    /**
     * @brief Construct an instance of the class.
     * @param [in] i2c driver instance
     * @return N/A
     */
    SM9333(I2CArbiter& i2c);

    /**
     * @brief Class instance destructor
     * @param N/A
     * @return N/A
     */
    ~SM9333();

    /**
     * @brief Initial command for checking if the device is measuring values
     *  reads serial number and stores them in private variable
     * @param N/A
     * @return
     * -    ESP_OK -> Sensor is alive and ready
     * -    ESP_FAIL -> Sensor is not ready or not working
     */
    esp_err_t init(void);

    /**
     * @brief Command to enter sleep mode on SM9333
     * @param N/A
     * @return Error case
     */
    esp_err_t sleepMode(void);

    /**
     * @brief Command to wake SM9333 from sleep mode
     * @param N/A
     * @return Error case
     */
    esp_err_t wakeUp(void);

    /**
     * @brief Command to reset the sensor; run init after the reset to asure the sensor is ready
     * @param N/A
     * @return Error case
     */
    esp_err_t reset(void);

    /**
     * @brief gets serial number of sensor - if empty, run init first
     * @param N/A
     * @return Serial number
     */
    uint32_t getSerial(void);

    /**
     * @brief gets the locally stored status value; use after measure_temperature() and measure_pressure()
     * @return last read status value
     */
    uint16_t getStatus(void);

    /**
     * @brief reads the status from the sensor
     * @param [out] dat: 16 bit data variable
     * @return Error case
     * 
     */
    esp_err_t readStatus(uint16_t *dat);
    
    /**
     * @brief command to reset event flags in status
     * @param N/A
     * @return error case
     */
    esp_err_t resetStatusFlags(void);

    /**
     * @brief get locally stored measured data for temperature and pressure; use after measure_all()
     * @param [out] temp: last valid temperature value
     * @param [out] pressure: last valid pressure value
     */
    void getMeasuredData(float *temp, float *pressure);
        
    /**
     * @brief get measured data from sensor and stores the valid values locally; use before get_measured_data()
     * @param N/A
     * @return Error case
     */
    esp_err_t measureAll(void);

    /**
     * @brief measures the temperature and checks if value is valid
     *  call get_status() after this, if wanted
     * @param [out] temperature in °C
     * @return Error case
     */
    esp_err_t measureTemperature(float *temperature);

    /**
     * @brief measures the pressure and checks if value is valid
     *  call get_status() after this, if wanted
     * @param [out] pressure value in Pa
     * @return Error case
     */
    esp_err_t measurePressure(float *pressure);

private:
    I2CArbiter&             m_i2c;              /*!< extern i2c interface */

    float                   m_temperature{0.0};     /*!< Last measured temperature*/
    float                   m_pressure{0.0};        /*!< Last measured pressure */
    bool                    m_temp_ready{false};    /*!< Temperature mesurement ready */
    bool                    m_press_ready{false};   /*!< Pressure measurement ready */
    uint16_t                m_status{0};            /*!< Sensor status */
    uint32_t                m_serial{0};            /*!< Sensor serial */

    /**
     * @brief write a single 16-bit register to the SM9333
     * @param [in] reg: register address
     * @param [in] dat: data to write in the register
     * @return Error case
     */
    esp_err_t writeRegister(uint8_t reg, uint16_t dat);

    /**
     * @brief read a single 16-bit register from SM9333
     * @param [in] reg: register address
     * @param [out] dat: data variable for response
     * @return Error case
     */
    esp_err_t readRegister(uint8_t reg, uint16_t *dat);
};

#endif /* MAIN_DRIVER_SM9333_H_ */