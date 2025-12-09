/**
 * @brief
 *
 * @file ConfigurationManager.hpp
 * @author your name
 * @date 2018-05-03
 */

#ifndef CONFIGURATION_MANAGER_H_
#define CONFIGURATION_MANAGER_H_

#include "Definitions.hpp"
#include "NVS.hpp"
#include "ArduinoJson.h"

#include "EventLog.hpp"

typedef struct{
	std::string timezone; // "CET-1CEST,M3.5.0,M10.5.0/3" (has multiple entries)
	std::string location; // "Europe/Berlin" (specifies a specific location)
	bool enableSNTP;
	std::string sntpServer;
	bool enableTimeOutput;
}TimeConfiguration_t;

typedef struct{
	std::string name;
	std::string location;
}SystemConfiguration_t;

class ConfigurationManager {
public:

	ConfigurationManager(EventLog &eventLog);
	~ConfigurationManager();

	void factoryReset();
	bool getFactoryResetFlag(){return m_factoryResetFlag;}

	NVS& getFactoryNVS(){ return m_factory_nvs; }
	NVS& getSystemNVS(){ return m_system_nvs; }
	NVS& getAuthenticatorNVS(){ return m_authenticator_nvs; }
	NVS& getWifiNVS(){ return m_wifi_nvs; }
	NVS& getEthernetNVS(){ return m_ethernet_nvs; }
	NVS& getCloudNVS(){ return m_cloud_nvs; }
	NVS& getMqttNVS(){ return m_mqtt_nvs; }
	NVS& getAzureNVS(){ return m_azure_nvs; }
	NVS& getModbusTcpNVS(){ return m_modbusTcp_nvs; }
	NVS& getAwsNVS(){ return m_aws_nvs; }
	NVS& getExtensionNVS(){ return m_extension_nvs; }

	void loadTimeConfiguration(void);
	TimeConfiguration_t getTimeConfiguration(void);
	void setTimeConfiguration(TimeConfiguration_t newConf);

	void loadSystemConfiguration(void);
	SystemConfiguration_t getSystemConfiguration(void);
	void setSystemConfiguration(SystemConfiguration_t newConf);

	std::string getProductionDate();
	std::string getHardwareVersion();
	std::string getProductVersion();
	std::string getFirmwareVersion();
	std::string getModuleType();
	std::string getCoreVersion();
	JsonObject  getSystemInfo();
	std::string getFactoryExtensionData();

private:
	EventLog &m_eventLog;

	NVS	m_factory_nvs;
	NVS	m_system_nvs;
	NVS m_authenticator_nvs;
	NVS m_wifi_nvs;
	NVS m_ethernet_nvs;
	NVS m_cloud_nvs;
	NVS m_mqtt_nvs;
	NVS m_azure_nvs;
	NVS m_modbusTcp_nvs;
	NVS m_aws_nvs;
	NVS m_extension_nvs;

	TimeConfiguration_t m_timeConfiguration;
	SystemConfiguration_t m_systemConfiguration;

	bool m_factoryResetFlag{false};
};

#endif