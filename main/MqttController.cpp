/**
 * @brief
 *
 * @file MqttController.cpp
 * @author your name
 * @date 2018-07-19
 */

#include "Definitions.hpp"
#include "CloudController.hpp"
#include "MqttController.hpp"
#include "JsonRpcCallHandler.hpp"
#include "ExtensionController.hpp"
#include "HMIController.hpp"
#include "System.hpp"
#include "FreeRTOS.hpp"
#include "ArduinoJson.h"
#include "NVS.hpp"
#include "GeneralUtils.hpp"
#include "File.hpp"

#include <string>
#include "esp_log.h"

static char LOGTAG[] = "MqttController";

MqttController::MqttController(CloudController *cloudController, ConfigurationManager &configurationManager, HMIController &hmiController)
	: Task("MqttController", 8192, 5),
	  m_cloudController(cloudController),
	  m_configurationManager(configurationManager),
	  m_hmiController(hmiController)
{
	ESP_LOGI(LOGTAG, "Starting...");

	m_mqttHandler = new MyMqttHandler(this);
	m_mqtt.setEventHandler(m_mqttHandler);
	m_DeviceID = System::getDeviceID();

	//load config
	uint32_t active, enable_config, port, qos, ca_crt, client_crt = 0;
	m_conf.port = 0;
	m_conf.qos = 0;
	m_conf.active = false;
	m_conf.enable_config = false;
	m_conf.ca_crt = false;
	m_conf.client_crt = false;

	if (m_configurationManager.getMqttNVS().get("active", active) == ESP_OK)
		m_conf.active = (bool)active;
	if (m_configurationManager.getMqttNVS().get("protocol", &m_conf.protocol) != ESP_OK)
		m_conf.protocol = "";
	if (m_configurationManager.getMqttNVS().get("host", &m_conf.host) != ESP_OK)
		m_conf.host = "";
	if (m_configurationManager.getMqttNVS().get("port", port) == ESP_OK)
		m_conf.port = (uint32_t)port;
	if (m_configurationManager.getMqttNVS().get("qos", qos) == ESP_OK)
		m_conf.qos = (uint32_t)qos;
	if (m_configurationManager.getMqttNVS().get("username", &m_conf.username) != ESP_OK)
		m_conf.username = "";
	if (m_configurationManager.getMqttNVS().get("password", &m_conf.password) != ESP_OK)
		m_conf.password = "";
	if (m_configurationManager.getMqttNVS().get("ca_crt", ca_crt) == ESP_OK)
		m_conf.ca_crt = (bool)ca_crt;
	if (m_configurationManager.getMqttNVS().get("client_crt", client_crt) == ESP_OK)
		m_conf.client_crt = (bool)client_crt;
	if (m_configurationManager.getMqttNVS().get("enable_config", enable_config) == ESP_OK)
		m_conf.enable_config = (bool)enable_config;
	if (m_configurationManager.getMqttNVS().get("stateTopic", &m_conf.stateTopic) != ESP_OK)
		m_conf.stateTopic = MQTT_TOPIC_PREFFIX"/"+m_DeviceID+ "/" MQTT_DATA_TOPIC_SUFFIX;
	if (m_configurationManager.getMqttNVS().get("commandTopic", &m_conf.commandTopic) != ESP_OK)
		m_conf.commandTopic = MQTT_TOPIC_PREFFIX"/"+m_DeviceID+ "/" MQTT_COMMAND_TOPIC_SUFFIX;
	if (m_configurationManager.getMqttNVS().get("configTopic", &m_conf.configTopic) != ESP_OK)
		m_conf.configTopic = MQTT_TOPIC_PREFFIX"/"+m_DeviceID+ "/" MQTT_CONFIG_TOPIC_SUFFIX;

	Task::start();
}

MqttController::~MqttController(void){
	Task::stop();
	stopClient();
	FreeRTOS::sleep(1000);
	if(m_mqttHandler != nullptr) {
		delete m_mqttHandler;
		m_mqttHandler = nullptr;
	}
	m_hmiController.setLED3State(HMIController::LEDState::OFF);
}

void MqttController::run(void *data){
	while (1){

		// set HMI Controller state
		if (isClientConnected()) {
			m_hmiController.setLED3State(HMIController::LEDState::ON);
		}else if(m_conf.active){
			m_hmiController.setLED3State(HMIController::LEDState::BLINK_SLOW);
		}

		FreeRTOS::sleep(10); //Sleep for 10ms

		// MQTT should be active, but is not...
		if(!isClientRunning() && m_conf.active){
			startClient();
		}

		// MQTT is active, should not
		if(isClientRunning() && !m_conf.active){
			stopClient();
		}

		// MQTT is isConnected and nothing is subscibed
		if(isClientConnected() && !m_allSubscribed){
			m_allSubscribed = subscribeAll();
			if(m_allSubscribed){
				sendDeviceInfoMessage();
			}
		}

		// MQTT is active an subscription is done
		if(isClientConnected() && m_allSubscribed){
			handleMqttMessages();
		}
	}
}

void MqttController::parseInputData(const std::string &topic, const std::string &payload){
	// prevent DOS attack!
	if(m_mqttData.size() < 10) {
		m_mqttData.push_back(make_pair(topic,payload));
		ESP_LOGD(LOGTAG, "Received a MQTT message!");
	}else{
		//send overflow info message
		std::string outStr;
		DynamicJsonDocument doc(1024);
		doc["state"] = "buffer overflow";
		doc["deviceID"] = m_DeviceID;

		TimeConfiguration_t time_conf = m_configurationManager.getTimeConfiguration();
		if(time_conf.enableTimeOutput){
			doc["timestamp"] = System::getLocalTimestamp(time_conf.timezone);
			doc["utcTime"] = (uint32_t) System::getTime();
		}

		serializeJson(doc, outStr);
		m_mqtt.publish(MQTT_TOPIC_PREFFIX"/"+m_DeviceID+ "/" MQTT_INFO_TOPIC_SUFFIX, outStr, 0,  true);
	}
}

void MqttController::handleMqttMessages(){

	if(m_mqttData.size() > 0){
		std::string topic = std::get<0>(m_mqttData.front());
		std::string payload = std::get<1>(m_mqttData.front());
		m_mqttData.erase(m_mqttData.begin());

		//ESP_LOGI(LOGTAG, "Got Message: %s %s", topic.c_str(), payload.c_str());
		//Parse the last Messages
		if(topic == m_conf.configTopic && m_conf.enable_config == true){
			// execute JSON RPC command
			if(m_jsonRpcCallHandler != nullptr){
				ESP_LOGD(LOGTAG, "JsonRpcCall: %s'", payload.c_str());
				sendDeviceDataMessage("RESULT", m_jsonRpcCallHandler->parse(payload));
			}else ESP_LOGE(LOGTAG, "ERROR: m_jsonRpcCallHandler is empty!");
		}else if (false){
			// Generic SmartFit Command, etc..
			// add generic "over all simelar" commants here:

		}else{
			// Nothing above fits, so than it is an extention command!
			if(m_extensionController != nullptr){
				std::string toReplace= m_conf.commandTopic;//+"/";
				size_t pos = topic.find(toReplace);
				topic.replace(pos, toReplace.length(), "");
				m_extensionController->parseMQTT(topic, payload);
			}else ESP_LOGE(LOGTAG, "ERROR: m_extentionController is empty!");
		}
	}
}

void MqttController::setConfigurationData(MqttConfiguration_t newConfig){
	m_conf = newConfig;
	m_configurationManager.getMqttNVS().set("active", (uint32_t)m_conf.active);
	m_configurationManager.getMqttNVS().set("protocol", m_conf.protocol);
	m_configurationManager.getMqttNVS().set("host", m_conf.host);
	m_configurationManager.getMqttNVS().set("port", m_conf.port);
	m_configurationManager.getMqttNVS().set("qos", m_conf.qos);
	m_configurationManager.getMqttNVS().set("username", m_conf.username);
	m_configurationManager.getMqttNVS().set("password", m_conf.password);
	m_configurationManager.getMqttNVS().set("ca_crt", (uint32_t)m_conf.ca_crt);
	m_configurationManager.getMqttNVS().set("client_crt", (uint32_t)m_conf.client_crt);
	m_configurationManager.getMqttNVS().set("enable_config", (uint32_t)m_conf.enable_config);
	m_configurationManager.getMqttNVS().set("stateTopic", m_conf.stateTopic);
	m_configurationManager.getMqttNVS().set("commandTopic", m_conf.commandTopic);
	m_configurationManager.getMqttNVS().set("configTopic", m_conf.configTopic);
	m_configurationManager.getMqttNVS().commit();
	stopClient(); // to restart later on
}

MqttConfiguration_t MqttController::getConfigurationData(void){
	return m_conf;
}

void MqttController::startClient(void){
	if (!m_isRunning){
		ESP_LOGI(LOGTAG, "Starting MQTT Client");
		m_isRunning = true;

		if(m_conf.host != "" && m_conf.port != 0){
			ESP_LOGI(LOGTAG, "Protocol: %s, Host: %s, Port %d Actice: %d", m_conf.protocol.c_str(), m_conf.host.c_str(),  m_conf.port, (int)m_conf.active);

			//set host
			if(m_conf.protocol != "mqtt://" && m_conf.protocol != "mqtts://" &&
			m_conf.protocol != "ws://" && m_conf.protocol != "wss://"){
				m_isRunning = false;
				ESP_LOGE(LOGTAG, "Not allowed protocol!");
				return;
			}
			std::string uri = m_conf.protocol + m_conf.host + ":" + std::to_string(m_conf.port);
			m_mqtt.setUri(uri);

			//set last will
			std::string outStr;
			DynamicJsonDocument doc(1024);
			doc["state"] = "offline";
			doc["deviceID"] = m_DeviceID;
			serializeJson(doc, outStr);

			std::string lastWillTopic = MQTT_TOPIC_PREFFIX"/"+ m_DeviceID + "/" MQTT_INFO_TOPIC_SUFFIX;
			std::string lastWillMsg = outStr;
			m_mqtt.setLastWill(lastWillTopic, lastWillMsg, 0,  true);

			//Connect with username and password if both configured, connect without otherwise
			if ((m_conf.username == "") || (m_conf.password == "")){
				ESP_LOGI(LOGTAG, "Connecting without username and password...");
			}else{
				ESP_LOGI(LOGTAG, "Connecting with username and password... %s:%s",m_conf.username.c_str(), m_conf.password.c_str());

				m_mqtt.setAuthorization(m_conf.username, m_conf.password);
			}

			//connect with given certificates
			if(m_conf.ca_crt){
				ESP_LOGI(LOGTAG, "Connecting with given CA crt");
				m_ssl.setCaCertificate(File(SPIFLASH_PATH INTERNAL_PATH "/upload/mqtt_ca.crt"));
			}else{
				ESP_LOGI(LOGTAG, "Connecting without given CA crt");
				m_ssl.setCaCertificate("");
			}
			if(m_conf.client_crt){
				ESP_LOGI(LOGTAG, "Connecting with given Client CRT and KEY");
				m_ssl.setCertificate(File(SPIFLASH_PATH INTERNAL_PATH "/upload/mqtt.crt"));
				m_ssl.setKey(File(SPIFLASH_PATH INTERNAL_PATH "/upload/mqtt.key"));
			}else{
				ESP_LOGI(LOGTAG, "Connecting without given Client CRT and KEY");
				m_ssl.setCertificate("");
				m_ssl.setKey("");
			}
			m_mqtt.setCertificate(&m_ssl);

			if(m_mqtt.start() == ESP_OK) return;
			else m_isRunning = false;
		}

		m_isRunning = false;
		ESP_LOGE(LOGTAG,"Can't start MQTT client!");
	}
}

void MqttController::stopClient(void){
	if (m_isRunning){
		ESP_LOGI(LOGTAG, "Stopping MQTT Client");
		m_hmiController.setLED3State(HMIController::LEDState::BLINK_SLOW); // stop need a while, so set led before stop()
		m_mqtt.stop();
		m_mqttHandler->onStop();
		m_isRunning = false;
		m_allSubscribed = false;
		m_mqttData.clear();
	}
}

bool MqttController::subscribeAll(){
	int i = 0;
	ESP_LOGI(LOGTAG, "Unsubscribe # topic");

	m_mqtt.unsubscribe("#");
	while(!m_mqttHandler->isUnsubscribed()){
		FreeRTOS::sleep(100);
		i++;
		if(i>100) return false;
	}
	ESP_LOGI(LOGTAG, "Unsubscribe # topic -> done!");

	if(m_conf.commandTopic != ""){
		i = 0;
		std::string topic = m_conf.commandTopic+"/#";
		m_mqtt.subscribe(topic, m_conf.qos);
		while(!m_mqttHandler->isSubscribed()){
			FreeRTOS::sleep(100);
			i++;
			if(i>100) return false;
		}
		ESP_LOGI(LOGTAG, "Subscribe command topic '%s' -> done!", topic.c_str());
	}

	if(m_conf.configTopic != ""){
		i=0;
		std::string topic = m_conf.configTopic;
		m_mqtt.subscribe(topic, m_conf.qos);
		while(!m_mqttHandler->isSubscribed()){
			FreeRTOS::sleep(100);
			i++;
			if(i>100) return false;
		}
		ESP_LOGI(LOGTAG, "Subscribe config topic '%s' -> done!",topic.c_str());
	}

	return true;
}

void MqttController::sendDeviceInfoMessage(void){
	std::string outStr;
	DynamicJsonDocument doc(1024);
	doc["state"] = "online";
	doc["stateTopic"] = m_conf.stateTopic;
	doc["commandTopic"] = m_conf.commandTopic;
	if(m_conf.enable_config) doc["configTopic"] = m_conf.configTopic;
	
	doc["systemInfo"].set(m_configurationManager.getSystemInfo());

	TimeConfiguration_t time_conf = m_configurationManager.getTimeConfiguration();
	if(time_conf.enableTimeOutput){
		doc["timestamp"] = System::getLocalTimestamp(time_conf.timezone);
		doc["utcTime"] = (uint32_t) System::getTime();
	}

	serializeJson(doc, outStr);

	m_mqtt.publish(MQTT_TOPIC_PREFFIX"/"+m_DeviceID+ "/" MQTT_INFO_TOPIC_SUFFIX, outStr, m_conf.qos,  true);
}

int MqttController::sendDeviceDataMessage(const std::string &topic, const std::string &payload){
	if (m_allSubscribed){
		return m_mqtt.publish(m_conf.stateTopic+topic, payload, m_conf.qos);
	}
	
	ESP_LOGI(LOGTAG, "Send failed");
	return -1;
}

bool MqttController::isClientConnected(void){
	return (m_mqttHandler->isConnected() && isClientRunning());
}

void MqttController::prepareReconnect(){
	m_allSubscribed = false;
}


// EVENTHANDLER STARTS HERE
/* ###################################################################### */

MyMqttHandler::MyMqttHandler(void* data){
	m_mqttController = (MqttController*)data;
	m_connected = false;
	m_unsubscribed = false;
	m_subscribed = false;
}

MyMqttHandler::~MyMqttHandler(){
	//nothing to do here
}

void MyMqttHandler::onConnected(esp_mqtt_event_handle_t event){
	ESP_LOGD("MqttHandler", "onConnected");
	FreeRTOS::sleep(100);
	m_connected = true;
}

void MyMqttHandler::onDisconnected(esp_mqtt_event_handle_t event){
	ESP_LOGD("MqttHandler", "onDisconnected");
	m_connected = false;
	m_mqttController->prepareReconnect();
}

void MyMqttHandler::onData(esp_mqtt_event_handle_t event){
	std::string topic = event->topic;
	topic = topic.substr(0,event->topic_len);
	std::string payload = event->data;
	payload = payload.substr(0,event->data_len);
	m_mqttController->parseInputData(topic, payload);

}

void MyMqttHandler::onPublished(esp_mqtt_event_handle_t event){
	m_mqttController->getCloudController()->sendDeviceDataMessageSuccess(event->msg_id);
}

void MyMqttHandler::onUnsubscribed(esp_mqtt_event_handle_t event){
	m_unsubscribed = true;
}

void MyMqttHandler::onSubscribed(esp_mqtt_event_handle_t event){
	m_subscribed = true;
}

void MyMqttHandler::onError(esp_mqtt_event_handle_t event){
	m_connected = false;
	m_unsubscribed = false;
	m_subscribed = false;
	ESP_LOGD("MqttHandler", "onError");
}

void MyMqttHandler::onStop(){
	m_connected = false;
	m_unsubscribed = false;
	m_subscribed = false;
}

bool MyMqttHandler::isConnected(){
	return m_connected;
}

bool MyMqttHandler::isUnsubscribed(){
	if(m_unsubscribed){
		m_unsubscribed = false;
		return true;
	}
	return false;
}

bool MyMqttHandler::isSubscribed(){
	if(m_subscribed){
		m_subscribed = false;
		return true;
	}
	return false;
}

int MqttController::parseJsonrpcGet(JsonVariant& input, JsonObject& output){

	std::string state;
	if (isClientConnected()) state = "connected";
	else state = "disconnected";

	JsonObject result = output.createNestedObject("result");
	result["select"] = CloudController::MQTT;
	result["active"] = m_conf.active;
	result["state"] = state;
	result["protocol"] = m_conf.protocol;
	result["qos"] = m_conf.qos;
	result["host"] = m_conf.host;
	result["port"] = m_conf.port;
	result["username"] = m_conf.username;
	result["ca_crt"] = m_conf.ca_crt;
	result["client_crt"] = m_conf.client_crt;
	result["enable_config"] = m_conf.enable_config;
	result["deviceID"] = System::getDeviceID();
	result["stateTopic"] = m_conf.stateTopic;
	result["commandTopic"] = m_conf.commandTopic;
	result["configTopic"] = m_conf.configTopic;
	return 0; // means success
}

int MqttController::parseJsonrpcSet(JsonVariant& input, JsonObject& output){
	MqttConfiguration_t conf = getConfigurationData();

	JsonVariant active = input["active"];
	JsonVariant protocol = input["protocol"];
	JsonVariant host = input["host"];
	JsonVariant port = input["port"];
	JsonVariant qos = input["qos"];
	JsonVariant username = input["username"];
	JsonVariant password = input["password"];

	JsonVariant ca_crt = input["ca_crt"];
	JsonVariant client_crt = input["client_crt"];
	JsonVariant enable_config = input["enable_config"];

	JsonVariant stateTopic = input["stateTopic"];
	JsonVariant commandTopic = input["commandTopic"];
	JsonVariant configTopic = input["configTopic"];

	// these parameters MUST included in the request
	if(!active.isNull()) conf.active = active.as<bool>();
	if(!active.isNull()) conf.active = active.as<bool>();
	if(!protocol.isNull()) conf.protocol = protocol.as<std::string>();
	if(!host.isNull()) conf.host = host.as<std::string>();
	if(!port.isNull()) conf.port = (uint32_t) port.as<unsigned int>();
	if(!qos.isNull()) conf.qos = (uint32_t) qos.as<unsigned int>();
	if(!username.isNull()) conf.username = username.as<std::string>();
	if(!password.isNull()) conf.password = password.as<std::string>();
	if(!stateTopic.isNull()) conf.stateTopic = stateTopic.as<std::string>();
	if(!commandTopic.isNull()) conf.commandTopic = commandTopic.as<std::string>();

	//following parameters CAN be included in the request
	if(ca_crt.isNull()) conf.ca_crt = false;
	else conf.ca_crt = ca_crt.as<bool>();

	if(client_crt.isNull()) conf.client_crt = false;
	else conf.client_crt = client_crt.as<bool>();

	if(enable_config.isNull()) conf.enable_config = false;
	else {
		// enable_config is only allowed if secured protocoll is used
		if (conf.protocol == "mqtts://" || conf.protocol == "wss://"){
			conf.enable_config = enable_config.as<bool>();
		}else conf.enable_config = false;
	}

	// if enable_config is true -> configTopic is a MUST have parameter
	if(conf.enable_config == true){
		if(configTopic.isNull()) return JSONRPC_INVALID_PARAMETER;
		else conf.configTopic = configTopic.as<std::string>();
	}

	if(conf.protocol != "mqtt://" && conf.protocol != "mqtts://" &&
		conf.protocol != "ws://" && conf.protocol != "wss://"){
		return JSONRPC_INVALID_PARAMETER;
	}

	// if(!GeneralUtils::isIp(conf.host) && !GeneralUtils::isHostname(conf.host))
	// 	return JSONRPC_INVALID_PARAMETER;

	setConfigurationData(conf);
	return 0;
}