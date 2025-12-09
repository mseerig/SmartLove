/**
 * @file Azure.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-07-22
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "Azure.hpp"
#include "MQTT.hpp"
#include "System.hpp"
#include "GeneralUtils.hpp"
#include "SNTP.hpp"
#include "HMAC.hpp"

#include <esp_log.h>

extern const uint8_t digicert_baltimore_root_pem_start[]   asm("_binary_digicert_baltimore_root_pem_start");
extern const uint8_t digicert_baltimore_root_pem_end[]   asm("_binary_digicert_baltimore_root_pem_end");

static char TAG[] = "Azure";

/**
 * @brief Construct a new Azure:: Azure object
 *
 * @param mqtt
 */
Azure::Azure(MQTT& mqtt):m_mqtt(mqtt),m_ssl(new SSLUtils){

	m_ssl->setCaCertificate(std::string((const char*)digicert_baltimore_root_pem_start));
}

/**
 * @brief Destroy the Azure:: Azure object
 *
 */
Azure::~Azure(){
	delete m_ssl;
}

/**
 * @brief Configures the given mqtt client according to the given Primary Connection String (found in the Azure IoT Hub).
 *
 * @param primaryConnectionString
 * @param expiry lease time in hours
 * @return esp_err_t error handle
 */
esp_err_t Azure::configureClient(std::string primaryConnectionString, uint32_t expiry){

	std::string hostname = GeneralUtils::getNextDataItem("HostName=", ";", &primaryConnectionString);
	if(hostname != ""){
		m_conf.hostname = hostname;
		ESP_LOGD(TAG, "hostname: %s", m_conf.hostname.c_str());
	}else return ESP_ERR_INVALID_ARG;

	std::string device_id = GeneralUtils::getNextDataItem("DeviceId=", ";", &primaryConnectionString);
	if(device_id != ""){
		m_conf.device_id = device_id;
		ESP_LOGD(TAG, "device_id: %s", m_conf.device_id.c_str());
	}else return ESP_ERR_INVALID_ARG;

	primaryConnectionString = primaryConnectionString+";";
	std::string shared_access_key = GeneralUtils::getNextDataItem("SharedAccessKey=", ";", &primaryConnectionString);
	if(shared_access_key != ""){
		m_conf.shared_access_key = shared_access_key;
		//ESP_LOGD(TAG, "shared_access_key: %s", m_conf.shared_access_key.c_str());
	}else return ESP_ERR_INVALID_ARG;

	if(refreshSAS(expiry) != ESP_OK) return ESP_FAIL;

	m_conf.username = m_conf.hostname+"/"+m_conf.device_id;
	m_conf.uri = "mqtts://"+m_conf.hostname+":";
	m_conf.uri+= AZURE_DEFAULT_PORT;

	m_subscribeTopic = "devices/"+m_conf.device_id+"/messages/devicebound/#";
	m_publishTopic = "devices/"+m_conf.device_id+"/messages/events/";

	m_mqtt.setClientId(m_conf.device_id);
	m_mqtt.setUri(m_conf.uri);
	m_mqtt.setAuthorization(m_conf.username, m_conf.sas);
	m_mqtt.setCertificate(m_ssl);

	return ESP_OK;
}

/**
 * @brief Generate the Azure SAS token (Mqtt password) for this session.
 *
 * @param expiry lease time in hours
 * @return esp_err_t erro handle
 */
esp_err_t Azure::refreshSAS(uint32_t expiry){

	std::string uri = m_conf.hostname + "/"+"devices"+"/"+m_conf.device_id;
	std::replace(uri.begin(), uri.end(), ' ', '+');
	uri = GeneralUtils::uriEncode(uri);

	unsigned long long ttl = (long)System::getTime() + (expiry*60*60);

	//generate sing_key
	std::string sing_key = uri;
	sing_key += '\n' + std::to_string(ttl);

	//Decode key for HMAC calculation
	std::string en_key;
	GeneralUtils::base64Decode(m_conf.shared_access_key, &en_key);

	//calculate HMAC
	uint8_t hmac_result[32];
	HMAC hmac((unsigned char*) en_key.c_str(), en_key.length());
	hmac.update((unsigned char*)sing_key.c_str(), sing_key.length());
	hmac.getResultHash(hmac_result);

	//base64 encode from hmac hash
	std::string hmac_result_str = (char*)hmac_result;
	hmac_result_str = hmac_result_str.substr (0,32);
	std::string signature;
	GeneralUtils::base64Encode(hmac_result_str, &signature);

	// change encoding
	signature = GeneralUtils::uriEncode(signature);

	//collect date to final SAS
	m_conf.sas = "SharedAccessSignature ";
	m_conf.sas+= "sr=" + uri + "&";
	m_conf.sas+= "sig=" + signature + "&";
	m_conf.sas+= "se=" + std::to_string(ttl);
	ESP_LOGD(TAG, "new SAS: '%s'", m_conf.sas.c_str());

	return ESP_OK;
}

/**
 * @brief Returns the Azure specific subscribe topic
 *
 * @return std::string topic
 */
std::string Azure::getSubscribeTopic(){
	return m_subscribeTopic;
}

/**
 * @brief Returns the Azure specific publish topic
 *
 * @return std::string topic
 */
std::string Azure::getPublishTopic(){
	return m_publishTopic;
}

/**
 * @brief Returns the Azure specific configurations
 *
 * @return azure_config_t azure configurations
 */
azure_config_t Azure::getConfigurations(){
	return m_conf;
}