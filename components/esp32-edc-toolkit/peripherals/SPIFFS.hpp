/*
 * SPIFFS.hpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#ifndef _SPIFFS_HPP_
#define _SPIFFS_HPP_

#include <string>
#include <esp_err.h>
#include <esp_spiffs.h>

/**
 * @brief Super simple static class to mount the SPIFFS.
 */
class SPIFFS{
public:
	static esp_err_t mount(std::string label, std::string mountPath);

};

#endif //_SPIFFS_HPP_