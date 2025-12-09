/*
 * I2C.h
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#ifndef MAIN_I2C_H_
#define MAIN_I2C_H_
#include <stdint.h>
#include <sys/types.h>
#include <driver/i2c.h>
#include <driver/gpio.h>


/**
 * @brief Interface to %I2C functions.
 */
class I2C {
private:
	uint8_t          m_address;
	i2c_cmd_handle_t m_cmd;
	bool             m_directionKnown;
	gpio_num_t       m_sdaPin;
	gpio_num_t       m_sclPin;
	i2c_port_t       m_portNum;

public:
	/**
	 * @brief The default SDA pin.
	 */
	static const gpio_num_t DEFAULT_SDA_PIN = GPIO_NUM_NC;

	/**
	 * @brief The default Clock pin.
	 */
	static const gpio_num_t DEFAULT_CLK_PIN = GPIO_NUM_NC;

	/**
	 * @brief The default Clock speed.
	 */
	static const uint32_t DEFAULT_CLK_SPEED = 100000;

	I2C();
	esp_err_t beginTransaction();
	esp_err_t endTransaction();
	uint8_t getAddress() const;
	esp_err_t init(uint8_t address, gpio_num_t sdaPin = DEFAULT_SDA_PIN, gpio_num_t sclPin = DEFAULT_CLK_PIN, uint32_t clkSpeed = DEFAULT_CLK_SPEED, i2c_port_t portNum = I2C_NUM_0);
	esp_err_t read(uint8_t* bytes, size_t length, i2c_ack_type_t ack=I2C_MASTER_ACK);
	esp_err_t read(uint8_t* byte, i2c_ack_type_t ack=I2C_MASTER_ACK);
	void scan();
	void setAddress(uint8_t address);
	void setDebug(bool enabled);
	bool slavePresent(uint8_t address);
	esp_err_t start();
	esp_err_t stop();
	esp_err_t write(uint8_t byte, i2c_ack_type_t ack=I2C_MASTER_ACK);
	esp_err_t write(uint8_t* bytes, size_t length, i2c_ack_type_t ack=I2C_MASTER_ACK);
	i2c_port_t getPortNum() const {return m_portNum;}
};

#endif /* MAIN_I2C_H_ */
