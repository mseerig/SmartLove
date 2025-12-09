/* MQTT.hpp
 *
 *
 *  Created on: 26.11.2018
 *      Author: Marcel Seerig
 *
 */

#ifndef COMPONENTS_CPP_UTILS_MQTT_HPP_
#define COMPONENTS_CPP_UTILS_MQTT_HPP_

#include "SSLUtils.hpp"
#include <string>
#include "mqtt_client.h"

class MQTTeventHandler{
	public:
		MQTTeventHandler(void* data=nullptr);
		virtual ~MQTTeventHandler();

		virtual void onData(esp_mqtt_event_handle_t event);
		virtual void onConnected(esp_mqtt_event_handle_t event);
		virtual void onDisconnected(esp_mqtt_event_handle_t event);
		virtual void onSubscribed(esp_mqtt_event_handle_t event);
		virtual void onUnsubscribed(esp_mqtt_event_handle_t event);
		virtual void onPublished(esp_mqtt_event_handle_t event);
		virtual void onError(esp_mqtt_event_handle_t event);
		virtual void onBeforeConnect(esp_mqtt_event_handle_t event);

	private:
		void* m_data;
};

class MQTT{
	public:
		MQTT();
		MQTT(std::string uri);
		MQTT(esp_mqtt_transport_t protocol, std::string host, int port);
		~MQTT();

		void setUri(std::string uri);
		void setHost(esp_mqtt_transport_t protocol, std::string host, int port);
		void setClientId(std::string client_id);
		void setAuthorization(std::string user, std::string pass);
		void setCertificate(SSLUtils* cert);
		void setLastWill(std::string topic, std::string payload, int qos = 0, bool retained = false);
		void setEventHandler(MQTTeventHandler* handler);

		esp_err_t start();
		esp_err_t stop();
		esp_err_t restart();

		int publish(std::string topic, std::string data, int qos=0, bool retain=false);
		esp_err_t subscribe(std::string topic, int qos=0);
		esp_err_t unsubscribe(std::string topic);

	private:
		static esp_err_t eventHandler(esp_mqtt_event_handle_t event);

		bool 					m_clientRunning{false};

		std::string 			m_uri={""};
		esp_mqtt_transport_t 	m_protocol={MQTT_TRANSPORT_UNKNOWN};
		std::string 			m_host={""};
		int 					m_port={0};
		std::string				m_client_id={""};
		std::string				m_username={""};
		std::string				m_password={""};
		SSLUtils*				m_cert={nullptr};
		std::string				m_lastwillTopic={""};
		std::string				m_lastWillMsg={""};
		bool					m_lastWillRetain={false};
		int						m_lastWillQos={0};

		esp_mqtt_client_handle_t m_client{NULL};
		MQTTeventHandler* 		m_handler;
};

#endif
