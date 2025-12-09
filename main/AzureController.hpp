/**
 * @file AzureController.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef CLOUD_CONTROLLER_HPP_
#define CLOUD_CONTROLLER_HPP_

#include "CloudController.hpp"
#include "ConfigurationManager.hpp"
#include "Definitions.hpp"
#include "HMIController.hpp"

/* IoT Hub library include */
#include "azure_iot_hub_client.h"
/* Transport interface implementation include header for TLS. */
#include "transport_tls_socket.hpp"
/* Crypto helper header. */
#include "azure_esp32_crypto.hpp"
/* Exponential backoff retry include. */
#include "backoff_algorithm.hpp"

#include "ArduinoJson.h"
#include "FreeRTOS.hpp"
#include "GeneralUtils.hpp"
#include "JSON_RPC.hpp"
#include "esp_log.h"
#include "timers.h"

#include <vector>
#include <string.h>

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    void * pParams;
};

typedef struct {
	bool active;
	std::string primaryConnectionString;
}AzureConfiguration_t;

class MyAzureHandler;
class JsonRpcCallHandler;
class ExtensionController;
class CloudController;

void reconnectTimerCallback(TimerHandle_t pTimer);

class AzureController:public Task {
public:
	AzureController(CloudController *cloudController, ConfigurationManager &configurationManager, HMIController &hmiController);
	~AzureController(void);

	int sendDeviceDataMessage(const std::string topic ,const std::string payload);

	void setConfigurationData(AzureConfiguration_t newConfig);
	AzureConfiguration_t getConfigurationData(void);

	void setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler){
		m_jsonRpcCallHandler = jsonRpcCallHandler;
	}

	void setExtensionController(ExtensionController* extensionController){
		m_extensionController = extensionController;
	}

	void parseInputData(const std::string &topic, const std::string &payload);
	void handleMqttMessages(void);

	int parseJsonrpcGet(JsonVariant& input, JsonObject& output);
	int parseJsonrpcSet(JsonVariant& input, JsonObject& output);

	std::string getHostname(std::string primaryConnectionString="");
	std::string getDeviceID(std::string primaryConnectionString="");
	std::string getSasKey(std::string primaryConnectionString="");

	CloudController* getCloudController(){return m_cloudController;}

	bool isConnectedToHubServer(){return m_IsConnectedToHubServer;}
	bool isHubClientInitialized(void){return m_IsInitialized;}
	bool isHubClientConnected(void){return m_IsHubClientConnected;}

private:
	void run(void *data);

	void connectToHubServer();
	void initHubClient();
	void connectHubClient();
	void abortStartup();
	
	void stopClient();
	void disconnectFromHubServer();
	void deinitHubClient();
	void disconnectHubClient();

	void reconnectDueToExpiredToken();

	static uint64_t ullGetUnixTime();
	uint32_t setupNetworkCredentials( NetworkCredentials_t * pxNetworkCredentials );
	void appendPropertyWihtErrorMessage(std::string key, std::string value);

	TimerHandle_t			m_disconnectforReconnectTimer;
	bool 					m_azureControllerTaskShouldStop{false};
	SemaphoreHandle_t 		m_taskShutdownDone;


	CloudController* 		m_cloudController;
	ConfigurationManager&	m_configurationManager;
	HMIController& 			m_hmiController;

	JsonRpcCallHandler*  	m_jsonRpcCallHandler;
	ExtensionController* 	m_extensionController;

	std::string				m_DeviceID;

	bool					m_IsConnectedToHubServer{false};
	bool					m_IsInitialized{false};
	bool					m_IsHubClientConnected{false};

	std::vector<std::pair<std::string,std::string>> m_azureData; //mqtt buffer

	//azure-iot-middleware-freertos variables
	AzureIoTResult_t 				m_xResult;
    uint32_t 						m_ulStatus;
	bool 							m_xSessionPresent{false};

	AzureIoTHubClient_t 			m_xAzureIoTHubClient;

	NetworkContext_t 				m_xNetworkContext = { 
										nullptr
    };
	 NetworkCredentials_t 			m_xNetworkCredentials = { 
										nullptr,            /**< @brief To use ALPN, set this to a NULL-terminated list of supported protocols in decreasing order of preference. */
										0,                  /**< @brief Disable server name indication (SNI) for a TLS session. */
										nullptr,            /**< @brief String representing a trusted server root certificate. */
										0,                  /**< @brief Size associated with #NetworkCredentials.pRootCa. */
										nullptr,            /**< @brief String representing the client certificate. */
										0,                  /**< @brief Size associated with #NetworkCredentials.pClientCert. */
										nullptr,            /**< @brief String representing the client certificate's private key. */
										0                   /**< @brief Size associated with #NetworkCredentials.pPrivateKey. */
    };

    AzureIoTTransportInterface_t 	m_xTransport;
    TlsTransportParams_t 			m_xTlsTransportParams = { 
										nullptr,
										nullptr
	};

    AzureIoTHubClientOptions_t 		m_xHubOptions = { 
										nullptr,            /**< The optional module ID to use for this device. */
										0,                  /**< The length of the module ID. */
										nullptr,            /**< The Azure Digital Twin Definition Language model ID used to
															*   identify the capabilities of this device based on the Digital Twin document. */
										0,                  /**< The length of the model ID. */
										nullptr,            /**< The list of component names to use for the device. */
										0,                  /**< The number of components in the list. */
										nullptr,            /**< The user agent to use for this device. */
										0,                  /**< The length of the user agent. */
										nullptr             /**< The callback to invoke to notify user a puback was received for QOS 1.
															*   Can be NULL if user does not want to be notified.*/
     };

    AzureIoTMessageProperties_t 	m_xPropertyBag;
    



	std::string 					m_iotHubHostname{""};
    std::string 					m_iotHubDeviceId{""};
	std::string						m_iotHubSasKey{""};
	std::string						m_iotHubModuleId{""};
	uint32_t 						m_iotHubPort{8883};			//TODO: maybe make available for customer

	uint8_t 						m_ucPropertyBuffer[ 1024 ];

	uint8_t 						m_ucMQTTMessageBuffer[ AzureIotNETWORK_BUFFER_SIZE ];	//buffer used to hold MQTT messages being sent and received.

	AzureConfiguration_t			m_conf;

};

#endif