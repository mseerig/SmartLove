/**
 * @file PWM.hpp
 * @author André Lange (andre.lange@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "PWM.hpp"

/**
 * @brief Construct an instance.
 *
 * A timer starts ticking up from 0 to the maximum value of the bit size.  When it reaches
 * the maximum value, it resets.
 *
 * In the following example, we create a signal that has a frequency of 20kHz and is
 * a square wave (75% on, 50% off).
 *
 * @code{.cpp}
 * PWM m_pwm(GPIO_NUM_4, 20000, LEDC_TIMER_8_BIT);
 * m_pwm.setDutyPercentage(75);
 * @endcode
 *
 * @param [in] gpioNum The GPIO pin to use for output.
 * @param [in] frequency The frequency of the period.
 * @param [in] dutyResolution The size in bits of the timer.  Allowed values are defined in @ref ledc_timer_bit_t.
 * @param [in] timer The timer to use. A value of LEDC_TIMER0, LEDC_TIMER1, LEDC_TIMER2 or LEDC_TIMER3 as defined in @ref ledc_timer_t.
 * @param [in] channel The channel to use.  A value from @ref ledc_channel_t.

 * @return N/A.
 */
PWM::PWM(gpio_num_t gpioNum, uint32_t frequency, ledc_timer_bit_t dutyResolution, ledc_timer_t timer, ledc_channel_t channel)
{
	m_initError = ESP_OK;

	ledc_timer_config_t timer_conf;
	timer_conf.duty_resolution = dutyResolution;
	timer_conf.freq_hz = frequency;
	timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
	timer_conf.timer_num = timer;
	timer_conf.clk_cfg = LEDC_AUTO_CLK;

	m_initError = ledc_timer_config(&timer_conf);

	if (m_initError == ESP_OK)
	{
		ledc_channel_config_t ledc_conf;
		ledc_conf.channel = channel;
		ledc_conf.duty = 0;
		ledc_conf.gpio_num = gpioNum;
		ledc_conf.intr_type = LEDC_INTR_DISABLE;
		ledc_conf.speed_mode = LEDC_LOW_SPEED_MODE;
		ledc_conf.timer_sel = timer;
		m_initError = ::ledc_channel_config(&ledc_conf);
	}

	m_gpio = gpioNum;
	m_channel = channel;
	m_timer = timer;
	m_dutyResolution = dutyResolution;
} // PWM

/**
 * @brief Get the frequency/period in Hz.
 *
 * @return The frequency/period in Hz.
 */
uint32_t PWM::getFrequency()
{
	return ::ledc_get_freq(LEDC_LOW_SPEED_MODE, m_timer);
} // getFrequency

/**
 * @brief Set the duty cycle value.
 *
 * The duty cycle value is a numeric between 0 and the maximum bit size.  When the
 * %PWM tick value is less than this value, the output is high while when it is higher
 * than this value, the output is low.
 * @param [in] duty The duty cycle value.
 * @return N/A.
 */
esp_err_t PWM::setDutyValue(uint32_t duty)
{
	esp_err_t error;
	error = ::ledc_set_duty(LEDC_LOW_SPEED_MODE, m_channel, duty);
	if (error == ESP_OK) {
		error = ::ledc_update_duty(LEDC_LOW_SPEED_MODE, m_channel);
	}
	return error;
} // setDuty

/**
 * @brief Set the duty cycle as a percentage value.
 *
 * @param [in] percent The percentage of the duty cycle (0-100).
 * @return N/A.
 */
esp_err_t PWM::setDutyPercentage(uint8_t percent)
{
	uint32_t max;

	max = 1 << m_dutyResolution;

	if (percent > 100)
		percent = 100;
	uint32_t value = max * percent / 100;
	if (value >= max)
		value = max - 1;
	return ( setDutyValue(value) );
} // setDutyPercentage

/**
 * @brief Set the frequency/period in Hz.
 *
 * @param [in] freq The frequency to set the %PWM.
 * @return N/A.
 */
esp_err_t PWM::setFrequency(uint32_t freq)
{
	return (::ledc_set_freq(LEDC_LOW_SPEED_MODE, m_timer, freq));
} // setFrequency

/**
 * @brief Stop the %PWM.
 *
 * @param [in] idleLevel The level to leave the output after end.
 * @return N/A.
 */
esp_err_t PWM::stop(bool idleLevel)
{
	return (::ledc_stop(LEDC_LOW_SPEED_MODE, m_channel, idleLevel ? 1 : 0));
} // stop