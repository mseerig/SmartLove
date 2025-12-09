/*
 * MQTT.cpp
 *
 *  Created on: 26.11.2018
 *      Author: Marcel Seerig
 *
 */

#include "MQTT.hpp"

#include "SSLUtils.hpp"

#include <string>
#include <mqtt_client.h>
#include <esp_log.h>

static const char *TAG = "MQTT";
static const char *EVENT = "MQTTeventHandler";

/**
 * @brief MQTT constructor.
 */
MQTT::MQTT(){
	m_handler = new MQTTeventHandler;
}

/**
 * @brief MQTT constructor.
 * @param esp_mqtt_transport_t specifies the socket type.
 * @param host specifies broker address.
 * @param post specifies broker port.
 */
MQTT::MQTT(std::string uri){
	m_handler = new MQTTeventHandler;
	m_uri = uri;
}

/**
 * @brief MQTT constructor.
 * @param esp_mqtt_transport_t specifies the socket type.
 * @param host specifies broker address.
 * @param post specifies broker port.
 */
MQTT::MQTT(esp_mqtt_transport_t protocol, std::string host, int port){
	m_handler = new MQTTeventHandler;
	m_host = host;
	m_protocol = protocol;
	m_port = port;
}

/**
 * @brief Set the client host credentials.
 * @param Uri formatted server parameters
 * @return N/A.
 */
void MQTT::setUri(std::string uri){
	m_uri = uri;
}

/**
 * @brief Set the client host credentials.
 * @param esp_mqtt_transport_t specifies the socket type.
 * @param host specifies broker address.
 * @param post specifies broker port.
 * @return N/A.
 */
void MQTT::setHost(esp_mqtt_transport_t protocol, std::string host, int port){
	m_host = host;
	m_protocol = protocol;
	m_port = port;
}

/**
 * @brief Set the client id for the MQTT client.
 * @param client id as string.
 * @return N/A.
 */
void MQTT::setClientId(std::string client_id){
	m_client_id = client_id;
}

/**
 * @brief Set the Authorization mode for the MQTT client.
 * @param username as string.
 * @param passphrase as string.
 * @return N/A.
 */
void MQTT::setAuthorization(std::string user, std::string pass){
	m_username = user;
    m_password = pass;
}

/**
 * @brief Set the certificates for the MQTT client.
 * @param SSL certificate as SSLUtils instance.
 * @return N/A.
 */
void MQTT::setCertificate(SSLUtils* cert){
	m_cert = cert;
}

/**
 * @brief Set the "last will" for the MQTT client.
 * @param topic as string.
 * @param payload as string.
 * @param qos level of the message.
 * @param retained, or not?.
 * @return N/A.
 */
void MQTT::setLastWill(std::string topic, std::string payload, int qos, bool retained){
	m_lastwillTopic = topic;
	m_lastWillMsg = payload;
	m_lastWillRetain = retained;
	m_lastWillQos = qos;
}

/**
 * @brief Set the extern event handler for the MQTT client.
 * @param pinter to the new extern event handler.
 * @return N/A.
 */
void MQTT::setEventHandler(MQTTeventHandler* handler){
	m_handler = handler;
}

/**
 * @brief Publish a message in the given topic.
 * @param [in] topic as string
 * @param [in] payload as string
 * @param [in] qos level (default is 0)
 * @param [in] retained, or not? (default is false)
 * @return value from esp_mqtt_client_publish().
 */
int MQTT::publish(std::string topic, std::string data, int qos, bool retain){
	return esp_mqtt_client_publish(m_client, topic.c_str(), data.c_str(), data.length(), qos, (int)retain);
}

/**
 * @brief Subscribe the given topic.
 * @param [in] topic as string
 * @param [in] qos level (default is 0)
 * @return N/A.
 */
esp_err_t MQTT::subscribe(std::string topic, int qos){
	return esp_mqtt_client_subscribe(m_client, topic.c_str(), qos);
}

/**
 * @brief Unsubscribe the given topic.
 * @param [in] topic as string
 * @return error case.
 */
esp_err_t MQTT::unsubscribe(std::string topic){
	return esp_mqtt_client_unsubscribe(m_client, topic.c_str());
}

/**
 * @brief Start the MQTT client.
 * @param N/A.
 * @return error case.
 */
esp_err_t MQTT::start(){
	esp_mqtt_client_config_t config = esp_mqtt_client_config_t();
	config.event_handle = eventHandler;
    config.user_context = m_handler;

	//set borker
	if(m_uri != ""){
		config.uri = m_uri.c_str();
		config.host = nullptr;
		config.port = 0;
		config.transport = MQTT_TRANSPORT_UNKNOWN;
	}else if (m_host != "" && m_port != 0 && m_protocol != MQTT_TRANSPORT_UNKNOWN){
		config.uri = nullptr;
		config.host = m_host.c_str();
		config.port = m_port;
		config.transport = m_protocol;
	}else return ESP_FAIL;

	//set client ID
	if(m_client_id != ""){
		config.client_id = m_client_id.c_str();
	}

	//set user and pass
	if(m_username != "" && m_password != ""){
		config.username = m_username.c_str();
		config.password = m_password.c_str();
	}

	//set Last Will
	if(m_lastWillMsg != "" && m_lastwillTopic != ""){
		config.lwt_topic = m_lastwillTopic.c_str();
		config.lwt_msg = m_lastWillMsg.c_str();
		config.lwt_qos = m_lastWillQos;
		config.lwt_retain = (int)m_lastWillRetain;
		config.lwt_msg_len = m_lastWillMsg.length();
	}
	if(m_cert != nullptr){
		if(m_cert->isCaCertificate()){
			config.cert_pem = (const char*) m_cert->getCaCertificate();
			//config.cert_len = sizeof(config.cert_pem);
		}

		if(m_cert->isCertificate() && m_cert->isKey()){
    		config.client_cert_pem = (const char*) m_cert->getCertificate();
			//config.cert_len = sizeof(config.client_cert_pem);
    		config.client_key_pem = (const char*) m_cert->getKey();
			//config.cert_len = sizeof(config.client_key_pem);
		}
	}

	//other defaults
	config.reconnect_timeout_ms = 10000;
	config.disable_clean_session = false;
	config.keepalive = 120; // 2 min
    config.disable_auto_reconnect = false;

	//redefine LOG level
	esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT", ESP_LOG_NONE);
	esp_log_level_set("OUTBOX", ESP_LOG_NONE);


	if(m_clientRunning) return ESP_FAIL;

	m_client = esp_mqtt_client_init(&config);

	//debug
	if(config.uri != nullptr){
		ESP_LOGD(TAG, "Start MQTT Client with target '%s'", config.uri);
	}else{
		ESP_LOGD(TAG, "Start MQTT Client with target '%s:%d'", config.host, config.port);
	}

	esp_err_t ret = esp_mqtt_client_start(m_client);
	if(ret == ESP_OK) m_clientRunning = true;

	return ret;
}

/**
 * @brief Stop the MQTT client.
 * @param N/A.
 * @return error case.
 */
esp_err_t MQTT::stop(void){
	if(m_clientRunning){
		m_clientRunning = false;
		return esp_mqtt_client_stop(m_client);
	}return ESP_OK;
}

/**
 * @brief Restart the MQTT client.
 * @param N/A.
 * @return error case.
 */
esp_err_t MQTT::restart(void){
	esp_err_t ret = stop();
	if(ret == ESP_OK) return ret = start();
	return ret;
}

/**
 * @brief Class destructor.
 */
MQTT::~MQTT(){
	esp_mqtt_client_destroy(m_client);
}

/**
 * @brief This is the static callback which parses the event in to the specific handler.
 */
esp_err_t MQTT::eventHandler(esp_mqtt_event_handle_t event){
	MQTTeventHandler * cb = reinterpret_cast<MQTTeventHandler*> (event->user_context);
	if(cb == nullptr) {
		ESP_LOGE(EVENT, "Got an esp_mqtt_event_handle_t but no handler found!");
		return ESP_FAIL;
	}
	switch(event->event_id){
		case MQTT_EVENT_ERROR:
			cb->onError(event);
			break;
    	case MQTT_EVENT_CONNECTED:
			cb->onConnected(event);
			break;
    	case MQTT_EVENT_DISCONNECTED:
			cb->onDisconnected(event);
			break;
    	case MQTT_EVENT_SUBSCRIBED:
			cb->onSubscribed(event);
			break;
    	case MQTT_EVENT_UNSUBSCRIBED:
			cb->onUnsubscribed(event);
			break;
    	case MQTT_EVENT_PUBLISHED:
			cb->onPublished(event);
			break;
    	case MQTT_EVENT_DATA:
			cb->onData(event);
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			cb->onBeforeConnect(event);
			break;
		default:
			ESP_LOGE(EVENT, "event_id unknown!");
			return ESP_FAIL;
	}
	return ESP_OK;
}

/**
 * @brief Class constructor of the MQTT event handler.
 * @param [in] your usecase speciffic data pointer.
 */
MQTTeventHandler::MQTTeventHandler(void* data):m_data(data){

}

/**
 * @brief Destroy the MQTTeventHandler::MQTTeventHandler object
 *
 */
MQTTeventHandler::~MQTTeventHandler(){

}

/**
 * @brief The onData callback function.
 * This Function is called, when a subscribed topic has new m_data.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onData(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onData()");
}

/**
 * @brief The onConnected callback function.
 * This Function is called, when the client successful connected to the broker.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onConnected(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onConnected()");
}

/**
 * @brief The onDisconnected callback function.
 * This Function is called, when the client disconnected to the broker.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onDisconnected(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onDisconnected()");
}

/**
 * @brief The onSubscribed callback function.
 * This Function is called, when the client subscribed a topic successful with qos>0.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onSubscribed(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onSubscribed()");
}

/**
 * @brief The onUnsubscribed callback function.
 * This Function is called, when the client unsubscribed a topic successful.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onUnsubscribed(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onUnsubscribed()");
}

/**
 * @brief The onPublished callback function.
 * This Function is called, when the client subscribed a topic published with qos>0.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onPublished(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onPublished()");
}

/**
 * @brief The onError callback function.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onError(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onError()");
}

/**
 * @brief The onBeforeConnect callback function.
 * @param The event handle with speciffic information.
 * @return N/A.
 */
void MQTTeventHandler::onBeforeConnect(esp_mqtt_event_handle_t event){
	ESP_LOGD(EVENT, "default onBeforeConnect()");
}