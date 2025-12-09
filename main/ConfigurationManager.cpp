/**
 * @brief
 *
 * @file ConfigurationManager.cpp
 * @author your name
 * @date 2018-05-03
 */

#include "Definitions.hpp"
#include "ConfigurationManager.hpp"
#include "System.hpp"

#include "esp_log.h"
#include "esp_err.h"

static char LOGTAG[]="ConfigurationManager";

ConfigurationManager::ConfigurationManager(EventLog &eventLog):
	m_eventLog(eventLog),
	m_factory_nvs(NVS("Factory")),
	m_system_nvs(NVS("System")),
	m_authenticator_nvs(NVS("Authenticator")),
	m_wifi_nvs(NVS("Wifi")),
	m_ethernet_nvs(NVS("Ethernet")),
	m_cloud_nvs(NVS("Cloud")),
	m_mqtt_nvs(NVS("Mqtt")),
	m_azure_nvs(NVS("Azure")),
	m_modbusTcp_nvs(NVS("ModbusTcp")),
	m_aws_nvs(NVS("AWS")),
	m_extension_nvs(NVS("Extension")){

		loadTimeConfiguration();
		loadSystemConfiguration();

	ESP_LOGI(LOGTAG, "Starting...");

}

ConfigurationManager::~ConfigurationManager(void) {

	ESP_LOGI(LOGTAG, "Exiting...");
}

void ConfigurationManager::factoryReset(void){
	ESP_LOGI(LOGTAG, "Factory Reset: Erase all!");
	m_system_nvs.erase();
	m_authenticator_nvs.erase();
	m_wifi_nvs.erase();
	m_ethernet_nvs.erase();
	m_cloud_nvs.erase();
	m_mqtt_nvs.erase();
	m_azure_nvs.erase();
	m_aws_nvs.erase();
	m_extension_nvs.erase();

	m_eventLog.clear();

	m_factoryResetFlag = true;
}

std::string ConfigurationManager::getProductionDate(){
	std::string productionDate;
	if((esp_err_t)m_factory_nvs.get("ProductionDate", &productionDate) != ESP_OK){
		return "unknown";
	}
	return productionDate;
}

std::string ConfigurationManager::getHardwareVersion(){
	std::string hardwareVersion;
	if((esp_err_t)m_factory_nvs.get("HardwareVersion", &hardwareVersion) != ESP_OK){
		return "unknown";
	}
	return hardwareVersion;
}

std::string ConfigurationManager::getProductVersion(){
	std::string hardwareVersion;
	if((esp_err_t)m_factory_nvs.get("ProductVersion", &hardwareVersion) != ESP_OK){
		// if not set -> Product version is default v1.0.0 (implemented since v1.0.1)
		return "v1.0.0";
	}
	return hardwareVersion;
}

/**
 * @brief This is the place for calibration Data calculated during endtest
 * 
 * @return std::string JSON Object as sting for more than one value
 */
std::string ConfigurationManager::getFactoryExtensionData(){
	std::string factoryData;
	if((esp_err_t)m_factory_nvs.get("ExtensionData", &factoryData) != ESP_OK){
		return "";
	}
	return factoryData;
}

std::string ConfigurationManager::getFirmwareVersion(){
	return FIRMWARE_VERSION;
}

std::string ConfigurationManager::getModuleType(){
	return MODULE_TYPE;
}

std::string ConfigurationManager::getCoreVersion(){
	return SMARTFIT_CORE_VERSION;
}

JsonObject ConfigurationManager::getSystemInfo(){
	DynamicJsonDocument doc(1024);
	JsonObject system_info = doc.createNestedObject("systemInfo");

	// set defaults if empty
	if(m_systemConfiguration.name == "") m_systemConfiguration.name=DEFAULT_NAME;
	if(m_systemConfiguration.location == "") m_systemConfiguration.location=DEFAULT_LOCATION;

	system_info["moduleType"] = getModuleType();
	system_info["name"] = m_systemConfiguration.name;
	system_info["location"] = m_systemConfiguration.location;
	system_info["productVersion"] = getProductVersion();
	system_info["deviceID"] = System::getDeviceID();
	system_info["productionDate"] = getProductionDate();
	system_info["firmwareVersion"] = getFirmwareVersion();
	system_info["hardwareVersion"] = getHardwareVersion();
	system_info["coreVersion"] = getCoreVersion();
	system_info["upTime"] = System::getUptime();
	return system_info;
}

void ConfigurationManager::loadTimeConfiguration(void){
	uint32_t buff=0;

	if (m_system_nvs.get("timezone", &m_timeConfiguration.timezone) != ESP_OK)
		m_timeConfiguration.timezone="UTC0";

	if (m_system_nvs.get("location", &m_timeConfiguration.location) != ESP_OK)
		m_timeConfiguration.location="Etc/UTC";

	m_timeConfiguration.enableSNTP = true;
	if (m_system_nvs.get("enableSNTP", buff) == ESP_OK)
		m_timeConfiguration.enableSNTP = static_cast<bool>(buff);

	if (m_system_nvs.get("sntpServer", &m_timeConfiguration.sntpServer) != ESP_OK)
		m_timeConfiguration.sntpServer="pool.ntp.org";

	m_timeConfiguration.enableTimeOutput = true;
	if (m_system_nvs.get("timeOutput", buff) == ESP_OK)
		m_timeConfiguration.enableTimeOutput = static_cast<bool>(buff);
}

TimeConfiguration_t ConfigurationManager::getTimeConfiguration(void){
	return m_timeConfiguration;
}

void ConfigurationManager::setTimeConfiguration(TimeConfiguration_t newConfig) {
	//Copy config content
	m_timeConfiguration = newConfig;

	//Store Config in NVS
	m_system_nvs.set("timezone", m_timeConfiguration.timezone);
	m_system_nvs.set("t_location", m_timeConfiguration.location);
	m_system_nvs.set("enableSNTP", m_timeConfiguration.enableSNTP);
	m_system_nvs.set("sntpServer", m_timeConfiguration.sntpServer);
	m_system_nvs.set("timeOutput", m_timeConfiguration.enableTimeOutput);
	m_system_nvs.commit();
}

void ConfigurationManager::loadSystemConfiguration(void){

	if (m_system_nvs.get("name1", &m_systemConfiguration.name) != ESP_OK)
		m_systemConfiguration.name=DEFAULT_NAME;

	if (m_system_nvs.get("name2", &m_systemConfiguration.location) != ESP_OK)
		m_systemConfiguration.location=DEFAULT_LOCATION;

}

SystemConfiguration_t ConfigurationManager::getSystemConfiguration(void){
	return m_systemConfiguration;
}

void ConfigurationManager::setSystemConfiguration(SystemConfiguration_t newConfig) {
	//Copy config content
	m_systemConfiguration = newConfig;

	//Store Config in NVS
	m_system_nvs.set("name1", m_systemConfiguration.name);
	m_system_nvs.set("name2", m_systemConfiguration.location);
	m_system_nvs.commit();
}