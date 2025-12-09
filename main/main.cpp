/**
 * @brief smartFIT Communication module main program
 *
 * @file main.cpp
 * @author andre.lange@ed-chemnitz.de
 * @date 2018-06-13
 */

#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <FreeRTOS.hpp>

#include "Definitions.hpp"
#include "ConfigurationManager.hpp"
#include "EventLog.hpp"
#include "NetworkController.hpp"
#include "CloudController.hpp"
#include "HMIController.hpp"
#include "WebserverController.hpp"
#include "JsonRpcCallHandler.hpp"
#include "ExtensionController.hpp"
#include "Authenticator.hpp"
#include "I2CArbiter.hpp"

#include "Memory.hpp"
#include "GeneralUtils.hpp"
#include "SysTick.hpp"
#include "Timer.hpp"
#include "SPIFFS.hpp"


/**
 * @brief Module specific LOGTAG for logging functionality
 *
 */
static char LOGTAG[]="Main";

/**
 * @brief Main entry point, C-declaration
 *
 */
extern "C" {
	void app_main(void);
}

void reduseLogLevel(void);
void mountSpiflash(void);

void app_main(void)
{
	reduseLogLevel();
	mountSpiflash(); // need to be first and "global"
	ESP_ERROR_CHECK(esp_netif_init()); // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create default event loop that running in background

	SysTick globalSysTick;

	I2CArbiter 				m_i2cArbiter;
	EventLog				m_eventLog(SPIFLASH_PATH "/log/system.log", EVENTLOG_LENGTH_MAX);
	ConfigurationManager 	m_configurationManager(m_eventLog);
	RtcController			m_rtcController(m_configurationManager, m_i2cArbiter, m_eventLog);

	m_eventLog.push(EventLog::Event::SYSTEM_POWER_UP, EventLog::State::INFO); // Straight after RTC init, push SYSTEM_POWER_UP log message

	HMIController			m_hmiController(m_configurationManager);
	Authenticator			m_authenticator(m_configurationManager);
	CloudController			m_cloudController(m_configurationManager, m_hmiController, m_eventLog);
	WebserverController		m_webserverController(m_authenticator, m_cloudController);
	NetworkController		m_networkController(m_configurationManager, m_hmiController, m_cloudController, m_eventLog);
	ExtensionController		m_extensionController(m_cloudController,m_networkController,m_configurationManager, m_hmiController, m_eventLog, m_i2cArbiter);
	JsonRpcCallHandler		m_jsonRpcCallHandler(m_cloudController, m_networkController, m_extensionController, m_authenticator, m_configurationManager, m_rtcController, m_eventLog);

	//set missing linkage
	m_cloudController.setJsonRpcCallHandler(&m_jsonRpcCallHandler);
	m_cloudController.setExtensionController(&m_extensionController);
	m_webserverController.setJsonRpcCallHandler(&m_jsonRpcCallHandler);
	m_webserverController.setExtensionController(&m_extensionController);

	ESP_LOGI(LOGTAG, "Starting...");

	m_hmiController.setLED1State(HMIController::LEDState::ON);

	while (1) {
		FreeRTOS::sleep(1042);		//Sleep for 1s
		Memory::dumpHeapChange("Main Loop");

		// every 1s
		m_webserverController.sendWebsocketBroadcast("bing");

		//Enable WIFI AP if requested
		if (!m_networkController.isWifiAccessPointActive()) {
			if (m_hmiController.userButtonWasPressed()) {
				m_networkController.requestWifiAPEnable();
			}
		}else{
			m_hmiController.setLED1State(HMIController::LEDState::BLINK_SLOW);
			if (m_hmiController.userButtonWasPressed()) {
				m_networkController.requestWifiAPDisable();
				m_hmiController.setLED1State(HMIController::LEDState::ON);
			}
		}

	}

	m_hmiController.setLED1State(HMIController::LEDState::OFF);
	m_hmiController.setLED2State(HMIController::LEDState::OFF);
	m_hmiController.setLED3State(HMIController::LEDState::OFF);

	ESP_LOGI(LOGTAG, "Exiting...");
	FreeRTOS::deleteTask();
}

void mountSpiflash(void){
	ESP_LOGI(LOGTAG, "Initializing SPIFFS");
	int ret = SPIFFS::mount(OtaController::getCurrentDataPartitionLabel(), SPIFLASH_PATH);
	ESP_LOGI(LOGTAG, "Initializing SPIFFS12");
	if (ret != ESP_OK)	{
		if (ret == ESP_FAIL){
			ESP_LOGE(LOGTAG, "Failed to mount or format filesystem");
		}else if (ret == ESP_ERR_NOT_FOUND)		{
			ESP_LOGE(LOGTAG, "Failed to find SPIFFS partition");
		}else{
			ESP_LOGE(LOGTAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		//ToDo: handle this error, brick device
	}
}

void reduseLogLevel(void){
	esp_log_level_set("WebserverController", ESP_LOG_DEBUG); // Kommentiere das aus und du siehst mehr Logs

	//esp_log_level_set("Memory", ESP_LOG_NONE);
	// esp_log_level_set("ExtensionController", ESP_LOG_NONE);

	//esp_log_level_set("jsonrpc_ws", ESP_LOG_NONE); // Kommentiere das aus und du siehst mehr Logs
	esp_log_level_set("spi_master", ESP_LOG_NONE);
	esp_log_level_set("bus_lock", ESP_LOG_NONE);
	esp_log_level_set("UART", ESP_LOG_NONE);
	esp_log_level_set("GPIOClass", ESP_LOG_NONE);

	esp_log_level_set("Socket", ESP_LOG_NONE);
	esp_log_level_set("HttpParser", ESP_LOG_NONE);
	esp_log_level_set("HttpServerTask", ESP_LOG_NONE);
	esp_log_level_set("HttpServerResponse", ESP_LOG_NONE);
	//esp_log_level_set("WiFiEventHandler", ESP_LOG_NONE);
	//esp_log_level_set("Ethernet", ESP_LOG_NONE);
	//esp_log_level_set("NetworkEventHandler", ESP_LOG_NONE);

	esp_log_level_set("w5500.mac", ESP_LOG_NONE);
	esp_log_level_set("HTTP_CLIENT", ESP_LOG_NONE);
	esp_log_level_set("tcpip_adapter", ESP_LOG_NONE);
	esp_log_level_set("wifi", ESP_LOG_NONE);
	esp_log_level_set("nvs", ESP_LOG_NONE);
	esp_log_level_set("efuse", ESP_LOG_NONE);

	esp_log_level_set("intr_alloc", ESP_LOG_NONE);
	esp_log_level_set("esp_netif_objects", ESP_LOG_NONE);
	esp_log_level_set("esp_netif_objects", ESP_LOG_NONE);
	esp_log_level_set("esp_netif_lwip", ESP_LOG_NONE);
	esp_log_level_set("esp_eth.netif.glue", ESP_LOG_NONE);
	esp_log_level_set("emac_esp32", ESP_LOG_NONE);
	esp_log_level_set("httpd", ESP_LOG_NONE);
	esp_log_level_set("httpd_sess", ESP_LOG_NONE);
	esp_log_level_set("httpd_parse", ESP_LOG_NONE);
	esp_log_level_set("httpd_txrx", ESP_LOG_NONE);
	esp_log_level_set("httpd_uri", ESP_LOG_NONE);
	esp_log_level_set("httpd_server", ESP_LOG_NONE);
	esp_log_level_set("httpd_ws", ESP_LOG_NONE);
}