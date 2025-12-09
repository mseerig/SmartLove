/**
 * @brief
 *
 * @file NetworkController.cpp
 * @author your name
 * @date 2018-07-10
 */

#include "Definitions.hpp"

#include "HMIController.hpp"

#include "NetworkController.hpp"
#include "MyNetworkEventHandler.hpp"
#include "Ethernet.hpp"
#include "WiFi.hpp"

#include "FreeRTOS.hpp"
#include "esp_log.h"

#include "GeneralUtils.hpp"
#include "NVS.hpp"
#include <string>

static char LOGTAG[] = "NetworkController";

NetworkController::NetworkController(ConfigurationManager &configurationManager, HMIController &hmiController, CloudController& cloudController, EventLog &eventLog)
	: Task("NetworkController", 4096, 5),
	  m_NetworkEventHandler(new MyNetworkEventHandler),
	  m_Ethernet(new Ethernet),
	  m_WiFi(new WiFi),
	  m_configurationManager(configurationManager),
	  m_hmiController(hmiController),
	  m_cloudController(cloudController),
	  m_eventLog(eventLog)
{

	ESP_LOGI(LOGTAG, "Starting...");

	loadEthernetConfiguration();
	loadWifiConfiguration();
	loadApConfiguration();

	Task::start();
}

NetworkController::~NetworkController(void)
{
}

/** load stored Data form NVS **/
void NetworkController::loadEthernetConfiguration(void){
	uint32_t active, useDHCP, user_dns=0;
	m_EthernetConfig.active = true;
	m_EthernetConfig.useDHCP = true;
	m_EthernetConfig.user_dns = false;

	if (m_configurationManager.getEthernetNVS().get( "active", active ) == ESP_OK)
		m_EthernetConfig.active = (bool)active;

	if (m_configurationManager.getEthernetNVS().get("useDHCP", useDHCP) == ESP_OK)
		m_EthernetConfig.useDHCP = static_cast<bool>(useDHCP);

	if (m_configurationManager.getEthernetNVS().get("staticIP", &m_EthernetConfig.ip) != ESP_OK)
		m_EthernetConfig.ip = "";

	if (m_configurationManager.getEthernetNVS().get("staticGateway", &m_EthernetConfig.gateway) != ESP_OK)
		m_EthernetConfig.gateway = "";

	if (m_configurationManager.getEthernetNVS().get("staticNetmask", &m_EthernetConfig.netmask) != ESP_OK)
		m_EthernetConfig.netmask = "";

	if (m_configurationManager.getEthernetNVS().get( "user_dns", user_dns) == ESP_OK)
		m_EthernetConfig.user_dns = (bool)user_dns;

	if (m_configurationManager.getEthernetNVS().get("dns_main", &m_EthernetConfig.dns_main) != ESP_OK)
		m_EthernetConfig.dns_main = "";

	if (m_configurationManager.getEthernetNVS().get("dns_fallback", &m_EthernetConfig.dns_fallback) != ESP_OK)
		m_EthernetConfig.dns_fallback = "";

	m_EthernetConfigChanged = true;
}

/** load stored Data form NVS **/
void NetworkController::loadWifiConfiguration(void){
	uint32_t active=0;
	m_WifiConfig.active = false;

	if (m_configurationManager.getWifiNVS().get("active", active) == ESP_OK)
		m_WifiConfig.active = static_cast<bool>(active);

	if (m_configurationManager.getWifiNVS().get("ssid", &m_WifiConfig.ssid) != ESP_OK)
		m_WifiConfig.ssid = "";

	if (m_configurationManager.getWifiNVS().get("password", &m_WifiConfig.password) != ESP_OK)
		m_WifiConfig.password = "";

	m_WifiConfigChanged = true;
}

/** load stored Data form NVS **/
void NetworkController::loadApConfiguration(void){
	uint32_t alwaysActive=0;
	m_ApConfig.tempActive = false;
	m_ApConfig.alwaysActive = false;
	if (m_configurationManager.getSystemNVS().get("ApAlwaysActive", alwaysActive) == ESP_OK)
		m_ApConfig.alwaysActive = static_cast<bool>(alwaysActive);

	if (m_configurationManager.getSystemNVS().get("ApSsid", &m_ApConfig.ssid) != ESP_OK)
		m_ApConfig.ssid = WIFI_AP_SSID;

	if (m_configurationManager.getSystemNVS().get("ApPassword", &m_ApConfig.password) != ESP_OK)
		m_ApConfig.password = WIFI_AP_PASS;

	m_ApConfigChanged = true;
}

/** Save only validated Data! **/
void NetworkController::setEthernetConfiguration(EthernetConfiguration_t newConfig) {
	//Copy config content
	m_EthernetConfig = newConfig;

	//Store Config in NVS
	m_configurationManager.getEthernetNVS().set("active", m_EthernetConfig.active );
	m_configurationManager.getEthernetNVS().set("useDHCP", m_EthernetConfig.useDHCP);
	m_configurationManager.getEthernetNVS().set("staticIP", m_EthernetConfig.ip);
	m_configurationManager.getEthernetNVS().set("staticGateway", m_EthernetConfig.gateway);
	m_configurationManager.getEthernetNVS().set("staticNetmask", m_EthernetConfig.netmask);
	m_configurationManager.getEthernetNVS().set("user_dns", m_EthernetConfig.user_dns );
	m_configurationManager.getEthernetNVS().set("dns_main", m_EthernetConfig.dns_main );
	m_configurationManager.getEthernetNVS().set("dns_fallback", m_EthernetConfig.dns_fallback );
	m_configurationManager.getEthernetNVS().commit();
	m_EthernetConfigChanged = true;
	
	m_eventLog.push(EventLog::Event::NETWORK_ETH_CONFIG_CHANGED, EventLog::State::INFO);
}

/** Save only validated Data! **/
void NetworkController::setWifiConfiguration(WifiConfiguration_t newConfig) {
	//Copy config content
	m_WifiConfig = newConfig;

	//Store Config in NVS
	m_configurationManager.getWifiNVS().set("active", m_WifiConfig.active);
	m_configurationManager.getWifiNVS().set("ssid", m_WifiConfig.ssid);
	m_configurationManager.getWifiNVS().set("password", m_WifiConfig.password);
	m_configurationManager.getWifiNVS().commit();
	m_WifiConfigChanged = true;

	m_eventLog.push(EventLog::Event::NETWORK_WIFI_CONFIG_CHANGED, EventLog::State::INFO);
}

/** Save only validated Data! **/
void NetworkController::setApConfiguration(ApConfiguration_t newConfig) {
	//Copy config content
	m_ApConfig = newConfig;

	//Store Config in NVS
	m_configurationManager.getSystemNVS().set("ApAlwaysActive", m_ApConfig.alwaysActive);
	m_configurationManager.getSystemNVS().set("ApSsid", m_ApConfig.ssid);
	m_configurationManager.getSystemNVS().set("ApPassword", m_ApConfig.password);
	m_configurationManager.getSystemNVS().commit();
	m_ApConfigChanged = true;
}

void NetworkController::handleClients(){
	//Start Webserver for Configuration if any network connection is active
	if ((isEthernetActive() || isWifiAccessPointActive() || isWifiStationActive()) &&
		(!m_EthernetConfigChanged && !m_WifiConfigChanged && !m_ApConfigChanged)) {
		if (!m_cloudController.isStarted()) m_cloudController.start();
	}else{
		if (m_cloudController.isStarted()) m_cloudController.stop();
	}
}

void NetworkController::run(void *data)
{
	Timer m_APTimeoutTimer;
	Timer wifiReconnect;

	while (1)
	{
		handleClients();

		//Set state of indicator LEDs according to Network status
		if ((isEthernetActive()) || (isWifiStationActive())){
			m_hmiController.setLED2State(HMIController::LEDState::ON);
		}else if (m_EthernetConfig.active || m_WifiConfig.active){
			m_hmiController.setLED2State(HMIController::LEDState::BLINK_SLOW);
		}

		//Adapt Configuration Changes for Ethernet
		if (m_EthernetConfigChanged) {
			handleClients();
			if (m_EthernetConfig.useDHCP) {
				m_Ethernet->setDynamicIP();
				ESP_LOGI(LOGTAG,"Ethernet set to dynamic IP");
			} else {
				m_Ethernet->setStaticIP(m_EthernetConfig.ip, m_EthernetConfig.gateway,  m_EthernetConfig.netmask);
				ESP_LOGI(LOGTAG,"Ethernet set to static IP(%s,%s,%s)",m_EthernetConfig.ip.c_str(), m_EthernetConfig.gateway.c_str(),m_EthernetConfig.netmask.c_str());
			}
			if(m_EthernetConfig.user_dns){ 
				m_Ethernet->setDnsInfo(ESP_NETIF_DNS_MAIN, m_EthernetConfig.dns_main);
				m_Ethernet->setDnsInfo(ESP_NETIF_DNS_FALLBACK, m_EthernetConfig.dns_fallback);
				// get DNS for debug
				ESP_LOGI(LOGTAG, "dns_main:     %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_MAIN).c_str());
				ESP_LOGI(LOGTAG, "dns_backup:   %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_BACKUP).c_str());
				ESP_LOGI(LOGTAG, "dns_fallback: %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_FALLBACK).c_str());
			}

			if (m_EthernetConfig.active) {
				enableEthernet();
			}else{
				disableEthernet();
			}

			m_EthernetConfigChanged = false;
		}

		//Adapt Configuration Changes for Wifi
		if (m_WifiConfigChanged) {
			handleClients();
			disableWifiStation(); //disable if active to apply changes in ssid and pass

			if (m_WifiConfig.active) {
				enableWifiStation(m_WifiConfig.ssid, m_WifiConfig.password);
				wifiReconnect.start_relative(TimerInterface::seconds(10));
			}
			m_WifiConfigChanged = false;
		}

		//Adapt Configuration for AP
		if(m_ApConfigChanged){
			handleClients();
			disableWifiAccessPoint(); //disable if active to apply changes in ssid and pass

			if(m_ApConfig.alwaysActive || m_ApConfig.tempActive){
				enableWifiAccessPoint(m_ApConfig.ssid, m_ApConfig.password);
			}
			m_ApConfigChanged = false;
		}

		//Adapt Configuration for SNTP
		if(m_SntpConfigChanged){
			m_sntp.stop(); //stop to apply changes

			m_sntp.setServer(m_configurationManager.getTimeConfiguration().sntpServer);
			if(m_configurationManager.getTimeConfiguration().enableSNTP){
				m_sntp.start();
			}
			m_SntpConfigChanged = false;
		}

		//Periodicaly check state of Ethernet und update variables
		if(m_EthernetEnabled) {
			if ((!m_EthernetWorking)&&(m_NetworkEventHandler->hasEthIP())) {
				m_EthernetWorking = true;
				m_EthernetIP = m_NetworkEventHandler->getEthernetIP();
				m_EthernetGateway = m_NetworkEventHandler->getEthernetGateway();
				m_SntpConfigChanged = true; // get also sntp time again

				if(m_EthernetConfig.user_dns){ 
					m_Ethernet->setDnsInfo(ESP_NETIF_DNS_MAIN, m_EthernetConfig.dns_main);
					m_Ethernet->setDnsInfo(ESP_NETIF_DNS_FALLBACK, m_EthernetConfig.dns_fallback);
				}
				// get DNS for debug
				ESP_LOGI(LOGTAG, "dns_main:     %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_MAIN).c_str());
				ESP_LOGI(LOGTAG, "dns_backup:   %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_BACKUP).c_str());
				ESP_LOGI(LOGTAG, "dns_fallback: %s", m_Ethernet->getDnsInfo(ESP_NETIF_DNS_FALLBACK).c_str());

			}
			if ((m_EthernetWorking)&&(!m_NetworkEventHandler->hasEthIP())) {
				m_EthernetIP = "";
				m_EthernetGateway = "";
				m_EthernetWorking = false;
				ESP_LOGI(LOGTAG,"Ethernet not working");
			}
			if ((m_EthernetWorking)&&(m_NetworkEventHandler->hasEthIP())) {
				m_EthernetIP = m_NetworkEventHandler->getEthernetIP();
				m_EthernetGateway = m_NetworkEventHandler->getEthernetGateway();
			}
		}else{
			m_EthernetWorking = false;
		}

		//Periodicaly check state of Wifi AP und update variables
		if (m_WifiAccessPointEnabled) {
			//Check if AP is working or not working
			if ((!m_WifiAccessPointWorking)&&(m_NetworkEventHandler->isAPstarted())) {
				m_WifiAccessPointWorking = true;
				m_APTimeoutTimer.start_relative(TimerInterface::minutes(1));
				ESP_LOGI(LOGTAG,"Wifi AP working");
			}
			if ((m_WifiAccessPointWorking)&&(!m_NetworkEventHandler->isAPstarted())) {
				m_WifiAccessPointWorking = false;
				ESP_LOGI(LOGTAG,"Wifi AP not working");
			}
			//Check if Client is connected to AP
			//Disable AP
			if (m_NetworkEventHandler->isStationConnected()) {
				m_APTimeoutTimer.start_relative(TimerInterface::minutes(1));
			}else{
				if (m_APTimeoutTimer.timeout() && !m_ApConfig.alwaysActive) {
					m_ApConfig.tempActive = false;
					m_ApConfigChanged = true;
				}
			}
		}else{
			m_WifiAccessPointWorking = false;
		}

		//Periodicaly check state of Wifi STA und update variables
		//ESP_LOGD(LOGTAG,"IP %d   working %d   stationEnabled %d",(int)m_NetworkEventHandler->hasStaIP(),(int)m_WifiStationWorking, (int)m_WifiStationEnabled);
		if (m_WifiStationEnabled) {
			if ((!m_WifiStationWorking)&&(m_NetworkEventHandler->hasStaIP())) {
				m_WifiStationWorking = true;
				m_WifiIP = m_WiFi->getStaIp();
				m_WifiGateway = m_WiFi->getStaGateway();
				m_SntpConfigChanged = true; // get also sntp again
				ESP_LOGI(LOGTAG,"Wifi STA working");
			}
			if ((m_WifiStationWorking)&&(m_NetworkEventHandler->hasStaIP())) {
				m_WifiIP = m_WiFi->getStaIp();
				m_WifiGateway = m_WiFi->getStaGateway();
			}
			if ((m_WifiStationWorking)&&(!m_NetworkEventHandler->hasStaIP())) {
				m_WifiStationWorking = false;
				m_WifiIP = "";
				m_WifiGateway = "";
				ESP_LOGI(LOGTAG,"Wifi STA not working");
				m_SntpConfigChanged = true; // get also sntp again
				wifiReconnect.start_relative(TimerInterface::seconds(10));
			}
			if((!m_WifiStationWorking)&&(!m_NetworkEventHandler->hasStaIP())){
				if(wifiReconnect.timeout()) {
					ESP_LOGI(LOGTAG,"DO A WIFI RECONNECT");
					m_WifiConfigChanged = true;
				}
			}
		}else{
			m_WifiStationWorking = false;
		}

		FreeRTOS::sleep(10); //Sleep for 10ms
	}
}

void NetworkController::enableEthernet(void)
{
	if (!m_EthernetEnabled)
	{
		//Enable Ethernet here
		ESP_LOGI(LOGTAG, "Enabling Ethernet...");

		int err = m_Ethernet->start();
		if (err == ESP_OK)
		{
			ESP_LOGI(LOGTAG, "Enabling Ethernet OK!");
			m_EthernetEnabled = true;
		}
		else
		{
			ESP_LOGE(LOGTAG, "Enabling Ethernet  FAILED! [Error: %d]", err);
		}
		
		if (m_EthernetConfig.useDHCP) {
			m_Ethernet->setDynamicIP();
			ESP_LOGI(LOGTAG,"Ethernet set to dynamic IP");
		} else {
			m_Ethernet->setStaticIP(m_EthernetConfig.ip, m_EthernetConfig.gateway,  m_EthernetConfig.netmask);
			ESP_LOGI(LOGTAG,"Ethernet set to static IP(%s,%s,%s)",m_EthernetConfig.ip.c_str(), m_EthernetConfig.gateway.c_str(),m_EthernetConfig.netmask.c_str());
		}
	}
}

void NetworkController::disableEthernet(void)
{
	if (m_EthernetEnabled)
	{
		//Disable Ethernet here
		ESP_LOGI(LOGTAG, "Disabling Ethernet...");

		int err = m_Ethernet->stop();
		if (err == ESP_OK)
		{
			ESP_LOGI(LOGTAG, "Disabling Ethernet OK!");
			m_EthernetEnabled = false;
		}
		else
		{
			ESP_LOGE(LOGTAG, "Disabling Ethernet  FAILED! [Error: %d]", err);
		}
	}
}

void NetworkController::enableWifiStation(std::string ssid, std::string passcode)
{
	if (!m_WifiStationEnabled)
	{
		ESP_LOGI(LOGTAG,"Starting STA");
		//Enable WIFI Station mode
		m_WifiStationEnabled = m_WiFi->startSTA(ssid, passcode);
	}
}

void NetworkController::disableWifiStation(void)
{
	if (m_WifiStationEnabled)
	{
		ESP_LOGI(LOGTAG,"Stopping STA");
		//Disable WIFI Station mode
		m_WiFi->stopSTA();
		m_WifiStationEnabled = false;
	}
}

void NetworkController::enableWifiAccessPoint(std::string ssid, std::string passcode)
{
	if (!m_WifiAccessPointEnabled)
	{
		ESP_LOGI(LOGTAG, "Starting AP");
		//Enable Wifi Access Point
		m_WifiAccessPointEnabled = m_WiFi->startAP(ssid,passcode,WIFI_AUTH_WPA2_PSK);
	}
}

void NetworkController::disableWifiAccessPoint(void)
{

	if (m_WifiAccessPointEnabled)
	{
		ESP_LOGI(LOGTAG, "Stopping AP");
		//Disable Wifi Access Point
		m_WiFi->stopAP();
		m_WifiAccessPointEnabled = false;
	}
}

void NetworkController::requestWifiAPEnable(void){
	if(m_ApConfig.alwaysActive) return;
	m_ApConfig.tempActive = true;
	m_ApConfigChanged = true;
}

void NetworkController::requestWifiAPDisable(void){
	if(m_ApConfig.alwaysActive) return;
	m_ApConfig.tempActive = false;
	m_ApConfigChanged = true;
}