/**
 * @brief
 *
 * @file MqttController.hpp
 * @author your name
 * @date 2018-07-19
 */

#ifndef MQTT_CONTROLLER_H_
#define MQTT_CONTROLLER_H_

#include "NetworkController.hpp"
#include "ConfigurationManager.hpp"
#include "HMIController.hpp"
#include "CloudController.hpp"

#include "FreeRTOS.hpp"
#include "MQTT.hpp"
#include "SSLUtils.hpp"
#include <vector>
#include <string>
#include "ArduinoJson.h"

typedef struct {
	bool active;
	std::string protocol;
	std::string host;
	uint32_t port;
	uint32_t qos;
	std::string username;
	std::string password;
	bool ca_crt;
	bool client_crt;
	bool enable_config;
	std::string stateTopic;
	std::string commandTopic;
	std::string configTopic;
}MqttConfiguration_t;

class MyMqttHandler;
class JsonRpcCallHandler;
class ExtensionController;
class CloudController;

class MqttController:public Task {
public:
	MqttController(CloudController *cloudController, ConfigurationManager &configurationManager, HMIController &hmiController);
	~MqttController(void);

	bool subscribeAll(void);
	void prepareReconnect();

	void sendDeviceInfoMessage(void);
	int sendDeviceDataMessage(const std::string& topic ,const std::string& payload);
	void parseInputData(const std::string &topic, const std::string &payload);

	bool isClientRunning(void){return m_isRunning;}
	bool isClientConnected(void);

	void handleMqttMessages(void);

	void setConfigurationData(MqttConfiguration_t newConfig);
	MqttConfiguration_t getConfigurationData(void);

	void setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler){
		m_jsonRpcCallHandler = jsonRpcCallHandler;
	}

	void setExtensionController(ExtensionController* extensionController){
		m_extensionController = extensionController;
	}

	int parseJsonrpcGet(JsonVariant& input, JsonObject& output);
	int parseJsonrpcSet(JsonVariant& input, JsonObject& output);

	void startClient(void);
	void stopClient(void);

	CloudController* getCloudController(){return m_cloudController;}

private:
	void run(void *data);
	bool					m_isRunning{false};
	bool					m_allSubscribed{false};
	MQTT					m_mqtt;
	SSLUtils				m_ssl;

	CloudController* 		m_cloudController;
	MyMqttHandler*			m_mqttHandler;
	ConfigurationManager&	m_configurationManager;
	HMIController& 			m_hmiController;
	std::string				m_DeviceID;

	std::vector<std::pair<std::string,std::string>> m_mqttData; //mqtt buffer

	MqttConfiguration_t	m_conf;

	JsonRpcCallHandler*  m_jsonRpcCallHandler;
	ExtensionController* m_extensionController;

};

class MyMqttHandler: public MQTTeventHandler{
	public:
		MyMqttHandler(void* data = nullptr);
		~MyMqttHandler();
		void onConnected(esp_mqtt_event_handle_t event);
		void onDisconnected(esp_mqtt_event_handle_t event);
		void onData(esp_mqtt_event_handle_t event);
		void onUnsubscribed(esp_mqtt_event_handle_t event);
		void onSubscribed(esp_mqtt_event_handle_t event);
		void onError(esp_mqtt_event_handle_t event);
		void onPublished(esp_mqtt_event_handle_t event);
		void onStop();
		bool isUnsubscribed();
		bool isSubscribed();
		bool isConnected();

	private:
		bool m_connected;
		bool m_unsubscribed;
		bool m_subscribed;
		MqttController* m_mqttController;
};


#endif