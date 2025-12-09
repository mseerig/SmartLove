/**
 * @file TCA6408A.hpp
 * @author Erik Friedel (erik.friedel@ed-chemnitz.de)
 * @brief Low-Voltage 8-Bit I2C and SMBus I/O Expander with interrupt Output, Reset, and Configuration Registers
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef MAIN_DRIVERS_TCA6408A_H_
#define MAIN_DRIVERS_TCA6408A_H_

#include <stdio.h>      /* printf, scanf */
#include <string>
#include <sys/time.h>

#include <esp_log.h>
#include <esp_err.h>
#include "I2CArbiter.hpp"

//Address has only 7bit, because the "least significant bit" is reserved for ACK/NAK.
#define TCA6408A_ADDR_OFFSET    0x01    /*!< represents the state of the physical "ADDR" Pin; High = 1, Low = 0*/
#define TCA6408A_BASE_ADDR      0x20    /*!< slave address for TCA6408A 0x20(L)/0x21(H)*/


//i2C
//#define TCA6408A_WRITE_BIT      0x00    /*!< I2C master write*/
//#define TCA6408A_READ_BIT       0x01    /*!< I2C master read*/
//#define TCA6408A_ACK_CHECK_EN   0x01    /*!< I2C master will check ack from slave*/
//#define TCA6408A_ACK_CHECK_DIS  0x00    /*!< I2C master will not check ack from slave*/
//#define TCA6408A_ACK_VAL        0x00    /*!< I2C ack value*/
//#define TCA6408A_NACK_VAL       0x01    /*!< I2C nack value*/

#define TCA6408A_INPUT          0x00    /*!< Input Data read only*/
#define TCA6408A_OUTPUT         0x01    /*!< Output Data read/write*/
#define TCA6408A_POLARITY       0x02    /*!< I/O polarity read/write*/
#define TCA6408A_CONFIG         0x03    /*!< I/O configuration read/write*/
                                        /*!< 0 = Output; 1 = Input HighZ*/


class TCA6408A {
public:
    /**
     * @brief Construct an instance of the class.
     * @param [in] i2c driver instance
     * @param [in] ADDR_PIN: the logical state of the ADDR PIN (High=1; Low=0)
     * @return N/A
     */
    TCA6408A(I2CArbiter& i2c, bool ADDR_PIN);

    /**
     * @brief Class instance destructor.
     * @param N/A
     * @return N/A
     */
    ~TCA6408A();

    /**
     * @brief Initial command for checking if the expander is alive
     * @param N/A
     * @return
     * -    ESP_OK -> Expander is alive.
     * -    ESP_FAIL -> Expander has a failure.
     */
    esp_err_t init(void);

    /**
     * @brief Configure Pin direction (I/O) and read polarity of a pin
     * @param [in] pin (0-7)
     * @param [in] direction: 0=Output, 1=Input
     * @param [in] pol: 0=Read data is not inverted, 1=Read data is inverted
     * @return Error case.
     */
    esp_err_t setupPinWrite(uint8_t pin, uint8_t direction, uint8_t pol);

    /**
     * @brief Configure direction (I/O) and read polarity; LSB=Pin0
     * @param [in] direction: 0=Output, 1=Input
     * @param [in] pol: 0=Read data is not inverted, 1=Read data is inverted
     * @return Error case.
     */
    esp_err_t setupPortWrite(uint8_t direction, uint8_t pol);

    /**
     * @brief Read Configuration of Pin direction (I/O) and read polarity
     * @param [in] pin (0-7)
     * @param [out] direction: 0=Output, 1=Input
     * @param [out] pol: 0=Read data is not inverted, 1=Read data is inverted
     * @return Error case.
     */
    esp_err_t setupPinRead(uint8_t pin, uint8_t *direction, uint8_t *pol);

    /**
     * @brief Read Configuration of port direction (I/O) and read polarity; LSB=Pin0
     * @param [out] direction: 0=Output, 1=Input
     * @param [out] pol: 0=Read data is not inverted, 1=Read data is inverted
     * @return Error case.
     */
    esp_err_t setupPortRead(uint8_t *direction, uint8_t *pol);

    /**
     * @brief Set state for output pin (has no effect on Input pins)
     * @param [in] pin: (0-7)
     * @param [in] state: True = Set, False = Reset
     * @return Error Case.
     */
    esp_err_t pinWrite(uint8_t pin, uint8_t state);

    /**
     * @brief Set state for output port (has no effect on Input pins); LSB=Pin0
     * @param [in] state: True = Set, False = Reset
     * @return Error Case.
     */
    esp_err_t portWrite(uint8_t state);

    /**
     * @brief Read state of input pin (also returns the set value for output pins)
     *  writing 1 to the corresponding polarity inversion register inverts the state.
     * @param [in] pin: (0-7)
     * @param [out] state: True = Set, False = Reset (if polarity inversion is not set)
     * @return Error Case.
     */
    esp_err_t pinRead(uint8_t pin, uint8_t *state);

    /**
     * @brief Read state of input pin (also returns the set value for output pins), LSB=Pin0
     *  writing 1 to the corresponding polarity inversion register inverts the state.
     * @param [out] state: True = Set, False = Reset (if polarity inversion is not set)
     * @return Error Case.
     */
    esp_err_t portRead(uint8_t *state);

private:
    I2CArbiter& m_i2c;      /*!<extern i2c instance */

    uint8_t m_i2cAddress;   /*!> I2C address*/

    /**
     * @brief write a single register to the TCA6408A
     * @param [in] reg: register address
     * @param [in] dat: data to write in the register
     * @return Error case.
     */
    esp_err_t writeRegister(uint8_t reg, uint8_t dat);

    /**
     * @brief read a single register from the TCA6408A
     * @param [in] reg: register address
     * @param [out] dat: data variable for response
     * @return Error case.
     */
    esp_err_t readRegister(uint8_t reg, uint8_t *dat);
};

#endif /* MAIN_DRIVER_TCA6408A_H_ */