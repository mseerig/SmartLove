/**
 * @file Azure.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief This class is parse and fill the credentials of a MQTT to connect to Microsoft Azure.
 * @version 0.1
 * @date 2019-07-22
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef COMPONENTS_ESP_TOOLKIT_AZURE_HPP_
#define COMPONENTS_ESP_TOOLKIT_AZURE_HPP_

#include "Azure.hpp"
#include "MQTT.hpp"
#include "SSLUtils.hpp"

#include <string>
#include "esp_err.h"

#define AZURE_DEFAULT_PORT 		"8883"

typedef struct{
	//input params
	std::string hostname;
	std::string device_id;
	std::string shared_access_key;
	//parse results
	std::string username;
	std::string uri;
	std::string sas;
}azure_config_t;

/**
 * @brief Class definition
 *
 */
class Azure{
	public:
	Azure(MQTT& mqtt);
	~Azure();

	esp_err_t configureClient(std::string primaryConnectionString, uint32_t expiry);
	esp_err_t refreshSAS(uint32_t expiry);

	std::string getSubscribeTopic();
	std::string getPublishTopic();

	azure_config_t getConfigurations();

	MQTT* getMQTT(){ return &m_mqtt; }

	private:
	MQTT& m_mqtt;
	SSLUtils* m_ssl;
	azure_config_t m_conf;

	std::string m_subscribeTopic{""};
	std::string m_publishTopic{""};
};

#endif
