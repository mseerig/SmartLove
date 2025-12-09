/**
 * @file CloudController.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-25
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef CLOUDCONTROLLER_HPP
#define CLOUDCONTROLLER_HPP

#include "MqttController.hpp"
#include "AzureController.hpp"
#include "ModbusTcpController.hpp"
#include "HMIController.hpp"
#include "ConfigurationManager.hpp"
#include "EventLog.hpp"

#include "FreeRTOS.hpp"
#include "Task.hpp"
#include "ArduinoJson.h"

#include <vector>
#include <tuple>

class MqttController;
class AzureController;
class ModbusTcpController;

class CloudController:public Task{
	public:

		static const std::string NONE;
		static const std::string MQTT;
		static const std::string AZURE;
		static const std::string MODBUSTCP_SERVER;

		CloudController(ConfigurationManager &configurationManager, HMIController &hmiController, EventLog &eventLog);
		~CloudController();

		void setClient(std::string select, bool commitNVS=true);
		void start(void);
		void stop(void);
		bool isStarted(void){return m_isStarted;}

		bool sendDeviceDataMessage(const std::string topic ,const std::string payload, bool useAsyncBuffer=false);
		void sendDeviceDataMessageSuccess(int msg_id);

		esp_err_t updateBinaryData(std::vector<uint16_t>& updateData);

		void setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler);
		void setExtensionController(ExtensionController* extensionController);

		int parseJsonrpcGet(JsonVariant& input, JsonObject& output);
		int parseJsonrpcSet(JsonVariant& input, JsonObject& output);

	private:
		
		std::string				m_select{NONE};
		bool 					m_isStarted{false};

		uint8_t					m_outputBufferSize{100}; // Allow 100 messages in output buffer
		bool 					m_bufferAccess{false};
		bool 					m_outputBufferActive{true};
		std::vector< std::tuple <int, std::string ,std::string> > outputBuffer;

		void run(void *data);
		void startMqttController();
		void startAzureController();
		void startModbusTcpController();
		void stopMqttController();
		void stopAzureController();
		void stopModbusTcpController();

		MqttController* 		m_mqttController{nullptr};
		AzureController*		m_azureController{nullptr};
		ModbusTcpController*    m_modbusTcpController{nullptr};

		JsonRpcCallHandler*  	m_jsonRpcCallHandler;
		ExtensionController* 	m_extensionController;

		ConfigurationManager&	m_configurationManager;
		HMIController& 			m_hmiController;
		EventLog& 				m_eventLog;

};

#endif