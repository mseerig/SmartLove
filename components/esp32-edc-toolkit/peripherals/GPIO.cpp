/**
 * @brief Simple GPIO-Klasse für statische Signale
 *
 * @file GPIO.cpp
 * @author andre.lange@ed-chemnitz.de
 * @date 2018-04-26
 */

#include "GPIO.hpp"
#include "esp_timer.h"

#include "esp_log.h"
static char LOGTAG[]="GPIOClass";

GPIOClass::GPIOClass(	gpio_num_t gpio, gpio_mode_t mode, gpio_pull_mode_t pullmode, gpio_drive_cap_t drivecap)
: m_gpio(gpio) {

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);

	ESP_LOGV(LOGTAG, "Setting up GPIO %d", m_gpio);
	gpio_set_direction( m_gpio, mode );
	ESP_LOGV(LOGTAG, "	Mode %d", mode);
	gpio_set_pull_mode( m_gpio,  pullmode );
	ESP_LOGV(LOGTAG, "	Pullup/down %d", pullmode);
	if (GPIO_IS_VALID_OUTPUT_GPIO(m_gpio)) {
		ESP_LOGV(LOGTAG, "	Drive %d", drivecap);
		gpio_set_drive_capability( m_gpio, drivecap );
	}
}

void GPIOClass::set(void)  {
	m_outputLevel = 1;
	gpio_set_level( m_gpio, 1);
}

void GPIOClass::reset(void) {
	m_outputLevel = 0;
	gpio_set_level( m_gpio, 0);
}

/**
 * @brief Toggles the GPIO pin
 * 
 */
void GPIOClass::toggle(void) {
	bool l = static_cast<bool>(m_outputLevel);
	write(static_cast<uint32_t>( !l ));
}

/**
 * @brief Function to let Blink an LED. The LED state is updated on each call. The toggle is then decided
 * based on the time difference to the last toggle and the given interval_ms. 
 * 
 * @param interval_ms time while LED is ON, or OFF
 */
void GPIOClass::pseudoBlink(uint16_t interval_ms){

	if(esp_timer_get_time()/ 1000 - m_lastBlink > interval_ms){
		m_lastBlink = esp_timer_get_time()/ 1000;
		toggle();
	}
}

uint8_t GPIOClass::read(void) {
	return static_cast<uint8_t>(gpio_get_level( m_gpio));
}

void GPIOClass::write(uint32_t level){
	m_outputLevel = level;
	gpio_set_level(m_gpio, level);
}