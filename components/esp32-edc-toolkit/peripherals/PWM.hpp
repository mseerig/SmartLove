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

#ifndef PWM_H_
#define PWM_H_
#include <driver/ledc.h>
/**
 * @brief A wrapper for ESP32S3 %PWM control.
 *
 * Pulse Width Modulation (%PWM) is known as "LEDC" in the ESP32S3.  It allows us to set a repeating
 * clock signal.  There are two aspects to the signal called the frequency and duty cycle.  The
 * frequency is how many times per second the signal repeats.  The duty cycle is how much of a single
 * period the output signal is high compared to low.  For example, a duty cycle of 0% means that the signal
 * is always low, while a duty cycle of 100% means the signal is always high.  A duty cycle of 50% means
 * that the signal is high for 50% of the output and low for the remaining 50%.
 */
class PWM {
public:
	PWM(
		gpio_num_t gpioNum,
		uint32_t frequency,
		ledc_timer_bit_t bitSize = LEDC_TIMER_8_BIT,
		ledc_timer_t timer       = LEDC_TIMER_0,
		ledc_channel_t channel   = LEDC_CHANNEL_0);

	/**
	 * @brief Get error code after initializaion of driver
	*/
	esp_err_t	getInitError(void){return m_initError;}
	uint32_t 	getFrequency();
	esp_err_t   setDutyPercentage(uint8_t percent);
	esp_err_t   setFrequency(uint32_t freq);
	esp_err_t   stop(bool idleLevel = false);


private:
	esp_err_t			m_initError;			///< Error state of PWM. Initialization in constructor can fail, user needs to be notified of this

	gpio_num_t	        m_gpio;				///< GPIO to use for PWM output
	ledc_channel_t      m_channel;			///< LEDC driver Channel
	ledc_timer_t        m_timer;			///< LEDC driver timer
	ledc_timer_bit_t    m_dutyResolution; 	///< Bit size of timer.


	esp_err_t	setDutyValue(uint32_t duty);	
};

#endif /* PWM_H_ */