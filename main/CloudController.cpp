/**
 * @file CloudController.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-25
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "MqttController.hpp"
#include "JsonRpcCallHandler.hpp"
#include "ExtensionController.hpp"
#include "HMIController.hpp"
#include "System.hpp"
#include "FreeRTOS.hpp"
#include "NVS.hpp"
#include "Memory.hpp"

#include <string>
#include "esp_log.h"

static char LOGTAG[] = "CloudController";

#define CLOUDCONTROLLER_MESSAGE_SEND_PENDING 	-999

const std::string CloudController::NONE		 			= "none";
const std::string CloudController::MQTT  				= "mqtt";
const std::string CloudController::AZURE 				= "azure";
const std::string CloudController::MODBUSTCP_SERVER 	= "modbustcp_s";

/**
 * @brief Construct a new Cloud Controller:: Cloud Controller object
 *
 * @param networkController
 * @param configurationManager
 * @param hmiController
 */
CloudController::CloudController(ConfigurationManager &configurationManager, HMIController &hmiController, EventLog &eventLog)
	: Task("CloudController", 16*1024, 5),
	  m_configurationManager(configurationManager),
	  m_hmiController(hmiController),
	  m_eventLog(eventLog)
{
	ESP_LOGI(LOGTAG, "Starting...");

	if(m_configurationManager.getCloudNVS().get("select", &m_select) != ESP_OK){
		//pre-define with none
		m_configurationManager.getCloudNVS().set("select", m_select);
		m_configurationManager.getCloudNVS().commit();
	}

	Task::start();
}

void CloudController::start(){
	if(!m_isStarted){
		m_isStarted = true;
		setClient(m_select, false); //set without commit selection to NVS
	}
}

void CloudController::stop(){
	if(m_isStarted){
		stopAzureController();
		stopMqttController();
		stopModbusTcpController();
		m_isStarted = false;
	}
}

void CloudController::setClient(std::string select, bool commitNVS){
	m_select = select; //for external call

	if(commitNVS){
		m_configurationManager.getCloudNVS().set("select", m_select);
		m_configurationManager.getCloudNVS().commit();
	}

	// Do nothing if start is not allowed!
	if(m_isStarted){

		if(m_select == CloudController::MQTT){
			stopAzureController();
			stopModbusTcpController();
			startMqttController();
			return;
		}
		if(m_select == CloudController::AZURE){
			stopMqttController();
			stopModbusTcpController();
			startAzureController();
			return;
		}
		if(m_select == CloudController::MODBUSTCP_SERVER){
			stopMqttController();
			stopAzureController();
			startModbusTcpController();
			return;
		}

		//any other -> disable all
		m_select = CloudController::NONE;
		stopMqttController();
		stopAzureController();
		stopModbusTcpController();
	}
}

void CloudController::startMqttController(){
	if(m_mqttController != nullptr) {
		ESP_LOGW(LOGTAG, "Mqtt Client still running.. Do nothing!");
		return;
	}
	m_mqttController = new MqttController(this, m_configurationManager, m_hmiController);
	m_mqttController->setJsonRpcCallHandler(m_jsonRpcCallHandler);
	m_mqttController->setExtensionController(m_extensionController);
}

void CloudController::stopMqttController(){
	if(m_mqttController != nullptr) delete(m_mqttController);
	m_mqttController = nullptr; //important!
}

void CloudController::startAzureController(){
	if(m_azureController != nullptr) {
		ESP_LOGW(LOGTAG, "Azure Client still running.. Do nothing!");
		return;
	}
	m_azureController = new AzureController(this, m_configurationManager, m_hmiController);
	m_azureController->setJsonRpcCallHandler(m_jsonRpcCallHandler);
	m_azureController->setExtensionController(m_extensionController);
}

void CloudController::stopAzureController(){
	if(m_azureController != nullptr) delete(m_azureController);
	m_azureController = nullptr; //important!
}

void CloudController::startModbusTcpController(){
	if(m_modbusTcpController != nullptr) {
		ESP_LOGW(LOGTAG, "Modbus Server still running.. Do nothing!");
		return;
	}
	m_modbusTcpController = new ModbusTcpController(this, m_configurationManager, m_hmiController);
}

void CloudController::stopModbusTcpController(){
	if(m_modbusTcpController != nullptr) delete(m_modbusTcpController);
	m_modbusTcpController = nullptr; //important!
}

/**
 * @brief Destroy the Cloud Controller:: Cloud Controller object
 *
 */
CloudController::~CloudController(void){

}

/**
 * @brief Sending a payload over an specific CloudController
 *
 * @param topic specify which data it is (see specific CloudController)
 * @param payload Payload to send over specific CloudController
 * @return true send success
 * @return false, if buffer is full. I no buffer is usesed, or buffer working, return true.
 */
bool CloudController::sendDeviceDataMessage(const std::string topic ,const std::string payload, bool useAsyncBuffer){
	// USING ASYNC BUFFER
	if( useAsyncBuffer && m_outputBufferActive){

		if(outputBuffer.size() < m_outputBufferSize){
			while(m_bufferAccess) FreeRTOS::sleep(1);
			m_bufferAccess = true;

			outputBuffer.push_back(std::make_tuple(CLOUDCONTROLLER_MESSAGE_SEND_PENDING, topic, payload)); // add message to output buffer

			m_bufferAccess = false;
			return true;
		}

		return false;
	}

	// DO NOT USE BUFFER, if not activated general, or not activated for this specific message
	// MQTT
	if(m_mqttController != nullptr && m_select == CloudController::MQTT){
		m_mqttController->sendDeviceDataMessage(topic, payload);
	}

	// AZURE
	if(m_azureController != nullptr && m_select == CloudController::AZURE){
		m_azureController->sendDeviceDataMessage(topic, payload);
	}

	return true;
	
}

void CloudController::sendDeviceDataMessageSuccess(int msg_id){
	ESP_LOGD(LOGTAG, "Message was sended with success: %d", msg_id);
	//find message in buffer
	while(m_bufferAccess) FreeRTOS::sleep(1);
	m_bufferAccess = true;
	
	for (int i = 0; i<outputBuffer.size(); i++) {
		//message found -> erase from buffer
        if(std::get<0>(outputBuffer.at(i)) == msg_id){
			outputBuffer.erase(outputBuffer.begin()+i);
			break; //otherwise incorrect iterator will occure errors
		}
    }

	m_bufferAccess = false;
}

esp_err_t CloudController::updateBinaryData(std::vector<uint16_t>& updateData){

	// MODBUS_TCP_SERVER
	if(m_modbusTcpController != nullptr && m_select == CloudController::MODBUSTCP_SERVER){
		return m_modbusTcpController->updateHoldingRegisters(updateData);
	}

	return ESP_ERR_NOT_FOUND;
}

void CloudController::run(void *data) {
	while(1){

		// do nothing, if buffer is empty
		if(outputBuffer.size() == 0){
			FreeRTOS::sleep(500);
			continue;
		}
		ESP_LOGD(LOGTAG, "Cloud Output Buffer size: %d", outputBuffer.size());

		// pop front of message buffer to get oldest message
		while(m_bufferAccess) FreeRTOS::sleep(1);
		m_bufferAccess = true;
		std::tuple <int, std::string ,std::string> message = outputBuffer.front();
		std::string topic = std::get<1>(message);
		std::string payload = std::get<2>(message);
		ESP_LOGD(LOGTAG, "id: %d -- topic: %s", std::get<0>(message), std::get<1>(message).c_str());
		outputBuffer.erase(outputBuffer.begin()); //clear this message
		m_bufferAccess = false;


		// TRY TO SEND MY CURRENT MESSAGE
		int ret = CLOUDCONTROLLER_MESSAGE_SEND_PENDING;

		// MQTT
		if(m_mqttController != nullptr && m_select == CloudController::MQTT){
			ret = m_mqttController->sendDeviceDataMessage(topic, payload);
		}

		// AZURE
		if(m_azureController != nullptr && m_select == CloudController::AZURE){
			ret = m_azureController->sendDeviceDataMessage(topic, payload);
		}

		ESP_LOGD(LOGTAG, "Send ret: %d", ret);
		//BUFFER
		// Return is 0 on QoS-Level 0, than ignore that message for buffer handling
		if(ret != 0){
			//Message is OoS-Level 1, or 2 -> add to buffer for futher handling
			outputBuffer.push_back(std::make_tuple(ret, topic, payload)); // add message to output buffer
		}

		// No Cloud
		if(m_mqttController == nullptr && m_azureController == nullptr){
			ESP_LOGW(LOGTAG, "all Clients are nullptr");
			FreeRTOS::sleep(5000); // delay wait for cloud
		}else{
			FreeRTOS::sleep(10); // delay for pushing whole buffer to cloud
		}
		
	}
}

/**
 * @brief Set JsonRpcCallHandler to access the JsonRpc Api from an specific CloudController.
 *
 * @param jsonRpcCallHandler pointer
 */
void CloudController::setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler){
	m_jsonRpcCallHandler = jsonRpcCallHandler;
}

/**
 * @brief Set Extention Controller for internal calls.
 *
 * @param extensionController pointer
 */
void CloudController::setExtensionController(ExtensionController* extensionController){
	m_extensionController = extensionController;
}

/**
 * @brief Passing the JSON_RPC Request to the active Controller.
 *
 * @param input
 * @param output
 * @return int returning JSONRPC_INTERNAL_ERROR is no controller active.
 */
int CloudController::parseJsonrpcGet(JsonVariant& input, JsonObject& output){
	if(m_mqttController != nullptr && m_select == CloudController::MQTT){
		return m_mqttController->parseJsonrpcGet(input, output);
	}

	if(m_azureController != nullptr && m_select == CloudController::AZURE){
		return m_azureController->parseJsonrpcGet(input, output);
	}

	if(m_modbusTcpController != nullptr && m_select == CloudController::MODBUSTCP_SERVER){
		return m_modbusTcpController->parseJsonrpcGet(input, output);
	}

	ESP_LOGW(LOGTAG, "all Clients are nullptr");
	JsonObject result = output.createNestedObject("result");
	result["select"] = m_select;
	return 0;
}

/**
 * @brief Passing the JSON_RPC Request to the active Controller.
 *
 * @param input
 * @param output
 * @return int returning JSONRPC_INTERNAL_ERROR is no controller active.
 */
int CloudController::parseJsonrpcSet(JsonVariant& input, JsonObject& output){
	JsonVariant select = input["select"];
	if(!select.isNull()) {
		setClient(select.as<std::string>());
		return 0;
	}

	if(m_mqttController != nullptr && m_select == CloudController::MQTT){
		return m_mqttController->parseJsonrpcSet(input, output);
	}

	if(m_azureController != nullptr && m_select == CloudController::AZURE){
		return m_azureController->parseJsonrpcSet(input, output);
	}

	if(m_modbusTcpController != nullptr && m_select == CloudController::MODBUSTCP_SERVER){
		return m_modbusTcpController->parseJsonrpcSet(input, output);
	}

	ESP_LOGW(LOGTAG, "all Clients are nullptr");
	JsonObject result = output.createNestedObject("result");
	result["select"] = "none";

	m_eventLog.push(EventLog::Event::CLOUD_CONFIG_CHANGED, EventLog::State::INFO);

	return 0;
}