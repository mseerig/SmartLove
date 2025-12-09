/*
 * ADC.hpp
 *
 *  Created on: 27.08.2018
 *      Author: marcel.seerig
 */

#ifndef _ADC_HPP_
#define _ADC_HPP_

#include "driver/adc.h"
#include "driver/gpio.h"

class ADC{
	public:
		ADC(adc_unit_t  unit, adc_channel_t channel);
		~ADC();

		int			getRAW();
		esp_err_t 	getGPIO(gpio_num_t *gpio_num);
		esp_err_t 	setDataWidth(adc_bits_width_t width_bit);
		esp_err_t 	setAttenuation(adc_atten_t atten);

	private:
		adc_unit_t  m_unit;
		adc_channel_t m_channel;
};

#endif //_ADC_HPP_