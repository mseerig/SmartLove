/*
 * ADC.cpp
 *
 *  Created on: 27.08.2018
 *      Author: marcel.seerig
 */

#include "ADC.hpp"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

ADC::ADC(adc_unit_t unit, adc_channel_t channel):m_unit(unit), m_channel(channel){
	//Configure ADC
    if (m_unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten((adc1_channel_t)channel, ADC_ATTEN_DB_0);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, ADC_ATTEN_DB_0);
    }
}

ADC::~ADC(){

}

int ADC::getRAW(){
	if (m_unit == ADC_UNIT_1) {
		return adc1_get_raw((adc1_channel_t) m_channel);
	} else {
		return -1; //not supportet jet!
	}
}

esp_err_t ADC::setDataWidth(adc_bits_width_t width_bit){
	return adc_set_data_width(m_unit, (adc_bits_width_t) width_bit);
}

esp_err_t ADC::setAttenuation(adc_atten_t atten){
	if (m_unit == ADC_UNIT_1) {
		return adc1_config_channel_atten((adc1_channel_t) m_channel, atten);
	} else {
		return adc2_config_channel_atten((adc2_channel_t) m_channel, atten);
	}
}

esp_err_t ADC::getGPIO(gpio_num_t *gpio_num){
	if (m_unit == ADC_UNIT_1) {
		return adc1_pad_get_io_num((adc1_channel_t) m_channel, gpio_num);
	} else {
		return adc2_pad_get_io_num((adc2_channel_t) m_channel, gpio_num);
	}
}

