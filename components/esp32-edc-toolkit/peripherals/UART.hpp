/**
 * @brief
 *
 * @file UART.hpp
 * @author your name
 * @date 2018-06-27
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
/**
 * @brief Configurations for UART
 */
typedef struct {
	uart_port_t num;
	int baud_rate; /*!< UART baudrate*/
	uart_word_length_t data_bits; /*!< UART byte size*/
	uart_parity_t parity; /*!< UART parity mode*/
	uart_stop_bits_t stop_bits; /*!< UART stop bits*/
	uart_hw_flowcontrol_t flow_ctrl; /*!< UART HW flow control mode(cts/rts)*/
	uint8_t rx_flow_ctrl_thresh; /*!< UART HW RTS threshold*/
	int tx_pin;
	int rx_pin;
	int rts_pin;
	int cts_pin;
	int rx_buffer_size;
	int tx_buffer_size;
} UART_InitTypeDef;


class UART {
public:
	UART(uart_port_t uartNum, int baud, gpio_num_t rxPin, gpio_num_t txPin);
	UART(const UART_InitTypeDef* uart_conf);
	~UART(void);

	bool hasData(void);
	uint8_t readByte(void);
	std::string readLine(void);
	void send(const uint8_t * data, const uint8_t size);
	void send(const uint8_t data);
	void send(const std::string data);
	esp_err_t flush(void);

private:
	uart_port_t uart_num;


};

#endif
