/**
 * @brief
 *
 * @file UART.cpp
 * @author your name
 * @date 2018-06-27
 */

#include "UART.hpp"

#include "esp_log.h"
#include "driver/uart.h"

static char LOGTAG[] = "UART";

UART::UART(uart_port_t uartNum, int baud, gpio_num_t rxPin, gpio_num_t txPin) : uart_num(uartNum)
{
	ESP_LOGI(LOGTAG, "Init...");
	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
	uart_config_t uart_config = {
		.baud_rate = baud,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 0,
		.use_ref_tick = false,
	};
	if (uart_param_config(uart_num, &uart_config) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_param_config failed");

	/* Configure UART pins

	 * Only set TX and RX pins, we dont use CTS and RTS
     */
	if (uart_set_pin(uart_num, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_set_pin failed");

	/* Install UART driver
	 *
	 * Use a 256 Byte RX ringbuffer
	 * No TX Buffer
	 * No event handling
	 */
	if (uart_driver_install(uart_num, 256, 0, 0, NULL, 0) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_driver_install failed");
}

UART::UART(const UART_InitTypeDef *uart_conf)
{
	ESP_LOGI(LOGTAG, "Init...");

	uart_num = uart_conf->num;

	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
	uart_config_t uart_config;
	uart_config.baud_rate = uart_conf->baud_rate;
	uart_config.data_bits = uart_conf->data_bits;
	uart_config.parity = uart_conf->parity;
	uart_config.stop_bits = uart_conf->stop_bits;
	uart_config.flow_ctrl = uart_conf->flow_ctrl;
	uart_config.rx_flow_ctrl_thresh = uart_conf->rx_flow_ctrl_thresh;

	if (uart_param_config(uart_num, &uart_config) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_param_config failed");

	/* Configure UART pins

	 * Only set TX and RX pins, we dont use CTS and RTS
     */
	if (uart_set_pin(uart_num, uart_conf->tx_pin, uart_conf->rx_pin,
					 uart_conf->rts_pin, uart_conf->cts_pin) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_set_pin failed");

	/* Install UART driver
	 *
	 * No event handling
	 */
	if (uart_driver_install(uart_num, uart_conf->rx_buffer_size, uart_conf->tx_buffer_size, 0, NULL, 0) != ESP_OK)
		ESP_LOGE(LOGTAG, "uart_driver_install failed");
}

UART::~UART(void)
{

	uart_driver_delete(uart_num);
}

bool UART::hasData(void)
{
	size_t dataLength;

	if (uart_get_buffered_data_len(uart_num, &dataLength) == ESP_OK)
	{
		if (dataLength > 0)
			return true;
	}

	return false;
}

uint8_t UART::readByte(void)
{
	uint8_t data;

	if (uart_read_bytes(uart_num, &data, 1, 20 / portTICK_RATE_MS) > 0)
	{
		return data;
	}
	return 0;
}

std::string UART::readLine(void)
{
	std::string data = "";
	uint8_t buff;

	while ((char)buff != '\n'){
		if (uart_read_bytes(uart_num, &buff, 1, 20 / portTICK_RATE_MS) > 0)	{
			data += (char) buff;
		}
	}
	
	return data;
}

void UART::send(const uint8_t data)
{
	ESP_LOGV(LOGTAG, "Send: %d", data);
	uart_write_bytes(uart_num, reinterpret_cast<const char *>(&data), 1);
	uart_wait_tx_done(uart_num, 1);
}

void UART::send(const uint8_t * data, const uint8_t size) {
	for (uint8_t len = 0; len < size; len++) {
		UART::send(data[len]);
	}
}


void UART::send(const std::string data)
{
	ESP_LOGV(LOGTAG, "Send: %s", data.c_str());
	int num = uart_write_bytes(uart_num, data.c_str(), data.size());
	uart_wait_tx_done(uart_num, 5);
	ESP_LOGV(LOGTAG, "%d bytes transmitted", num);
}

/**
 * @brief Clear input buffer, discard all the data is in the ring-buffer.
 * @note  In order to send all the data in tx FIFO, we can use uart_wait_tx_done function.
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t UART::flush(void){
	return uart_flush_input(uart_num);
}