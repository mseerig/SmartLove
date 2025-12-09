/**
 * @brief
 *
 * @file ExtensionController.hpp
 * @author Marcel Seerig
 * @date 2019-01-25
 */

#ifndef EXTENTION_CONTROLLER_H_
#define EXTENTION_CONTROLLER_H_



#include "HMIController.hpp"
#include "ConfigurationManager.hpp"
#include "CloudController.hpp"
#include "NetworkController.hpp"
#include "EventLog.hpp"
#include "ArduinoJson.h"
#include "Definitions.hpp"

#include "WebserverController.hpp"

#include "SysTick.hpp"
#include "Timer.hpp"
#include "FreeRTOS.hpp"
#include "Task.hpp"
#include <string>

#include "esp_http_server.h"


class CloudController;
class NetworkController;
class I2CArbiter;

class ExtensionController:public Task{
	public:
		ExtensionController(CloudController& cloudController, NetworkController& networkController, ConfigurationManager &configurationManager, HMIController &hmiController, EventLog &eventLog, I2CArbiter &i2cArbiter);
		~ExtensionController();

		void loadDefaultConfig();
		
		void updateModbusTCP(void);
		void parseMQTT(std::string topic, std::string payload);

		int parseJsonrpcGet(JsonVariant& input, JsonObject& output);
		int parseJsonrpcSet(JsonVariant& input, JsonObject& output);

		void prepareReboot(void);
		void factoryReset(void);

		esp_err_t httpDataHandler(httpd_req_t *req);
		uint8_t * getData(size_t chunk_size, uint16_t & len);
		uint8_t * getBufferData(uint16_t & len);



	private:
		void run(void *data);
	    
		DynamicJsonDocument 		m_conf{4096};				//config sorage for ExtensionController
		
		CloudController& 			m_cloudController;
		NetworkController& 			m_networkController;
		ConfigurationManager&		m_configurationManager;
		HMIController&				m_hmiController;
		EventLog&					m_eventLog;

};


#endif
