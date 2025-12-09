/**
 * @brief Simple GPIO-Klasse für statische Signale
 *
 * @file GPIO.hpp
 * @author andre.lange@ed-chemnitz.de
 * @date 2018-04-26
 */
#ifndef GPIO_HPP_
#define GPIO_HPP_

#include "driver/gpio.h"
#include "stdint.h"

class  GPIOClass{
public:
	/**
	 * @brief Construct a new GPIO object
	 * @note No error checking is performed
	 *
	 * @param gpio Number of GPIO as defined in gpio.h of esp-idf
	 * @param mode Mode of GPIO as defined in gpio.h of esp-idf
	 * @param pullmode Mode of internal pullup/pulldown resistors as defined in gpio.h of esp-idf
	 * @param drivecap Output driving capability as defined in gpio.h of esp-idf
	 */
	GPIOClass(	gpio_num_t gpio,
				gpio_mode_t mode = GPIO_MODE_INPUT,
				gpio_pull_mode_t pullmode = GPIO_PULLDOWN_ONLY,
				gpio_drive_cap_t drivecap= GPIO_DRIVE_CAP_0);

	/**
	 * @brief Set GPIO output high
	 */
	void set(void);

	/**
	 * @brief Set GPIO output low
	 *
	 */
	void reset(void);

	/**
	 * @brief Toggles the GPIO pin
	 * 
	 */
	void toggle(void);

	/**
	 * @brief Function to let Blink an LED. The LED state is updated on each call. The toggle is then decided
	 * based on the time difference to the last toggle and the given interval_ms. 
	 * 
	 * @param interval_ms time while LED is ON, or OFF
	 */
	void pseudoBlink(uint16_t interval_ms);

	/**
	 * @brief Read GPIO input status
	 *
	 * @return uint8_t 0 if input was low, 1 if input was high
	 */
	uint8_t read(void);

	/**
	 * @brief Write 1/0 directly to the gpio
	 * 
	 * @param level Logic level
	 */
	void write(uint32_t level);

	gpio_num_t getGpioNum(void) {return m_gpio;}

private:
	gpio_num_t	m_gpio;
	uint64_t m_lastBlink = 0;
	uint32_t m_outputLevel = 0; // uint32_t in idf for some reason
};

#endif