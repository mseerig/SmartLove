/**
 * @brief
 *
 * @file NetworkController.hpp
 * @author your name
 * @date 2018-07-10
 */
#ifndef NETWORK_CONTROLLER_H_
#define NETWORK_CONTROLLER_H_

#include "ConfigurationManager.hpp"
#include "HMIController.hpp"
#include "CloudController.hpp"
#include "EventLog.hpp"

#include "Task.hpp"
#include "Timer.hpp"
#include "SNTP.hpp"

#include <vector>
#include <string>

class MyNetworkEventHandler;
class WiFi;
class Ethernet;
class CloudController;
class WebserverController;

typedef struct {
	bool active;
	bool useDHCP;
	std::string ip;
	std::string gateway;
	std::string netmask;
	bool user_dns;
	std::string dns_main;
	std::string dns_fallback;
}EthernetConfiguration_t;

typedef struct {
	bool active;
	std::string ssid;
	std::string password;
}WifiConfiguration_t;

typedef struct {
	bool alwaysActive;
	bool tempActive;
	std::string ssid;
	std::string password;
}ApConfiguration_t;

class NetworkController: public Task {
  public:
	NetworkController(ConfigurationManager &configurationManager, HMIController &hmiController, CloudController& cloudController, EventLog &eventLog);
	~NetworkController(void);

	void requestWifiAPEnable(void);
	void requestWifiAPDisable(void);

	bool isEthernetActive(void){return m_EthernetWorking;}
	bool isWifiStationActive(void){return m_WifiStationWorking;}
	bool isWifiAccessPointActive(void){return m_WifiAccessPointWorking;}

	std::string getEthernetIP(void){return m_EthernetIP;}
	std::string getEthernetGateway(void){return m_EthernetGateway;}

	std::string getWifiIP(void){return m_WifiIP;}
	std::string getWifiGateway(void){return m_WifiGateway;}

	EthernetConfiguration_t getEthernetConfiguration(void) {return m_EthernetConfig;}
	WifiConfiguration_t getWifiConfiguration(void) {return m_WifiConfig;}
	ApConfiguration_t getApConfiguration(void) {return m_ApConfig;}

	void setEthernetConfiguration(EthernetConfiguration_t newConfig);
	void setWifiConfiguration(WifiConfiguration_t newConfig);
	void setApConfiguration(ApConfiguration_t newConfig);
	void setSntpConfigurationChanged(){m_SntpConfigChanged = true;};

	void loadEthernetConfiguration(void);
	void loadWifiConfiguration(void);
	void loadApConfiguration(void);

	bool doneEthernetChanges() {return !m_EthernetConfigChanged;}
	bool doneWifiChanges() {return !m_WifiConfigChanged;}
	bool doneApChanges() {return !m_ApConfigChanged;}
	bool doneSntpChanges() {return !m_SntpConfigChanged;}

  private:
	bool m_EthernetEnabled{false};
	bool m_WifiStationEnabled{false};
	bool m_WifiAccessPointEnabled{false};
	bool m_WifiAccessPointEnabledRequest{false};

	bool m_EthernetWorking{false};
	bool m_WifiStationWorking{false};
	bool m_WifiAccessPointWorking{false};

	std::string m_EthernetIP;
	std::string m_EthernetGateway;

	std::string m_WifiIP;
	std::string m_WifiGateway;

	MyNetworkEventHandler*  m_NetworkEventHandler;

	Ethernet*				m_Ethernet;
	WiFi*					m_WiFi;

	ConfigurationManager	&m_configurationManager;
	HMIController 			&m_hmiController;
	CloudController 		&m_cloudController;
	EventLog				&m_eventLog;

	bool					m_EthernetConfigChanged{false};
	bool					m_WifiConfigChanged{false};
	bool					m_ApConfigChanged{false};
	bool 					m_SntpConfigChanged{true};

	SNTP 					m_sntp;


	EthernetConfiguration_t	m_EthernetConfig;
	WifiConfiguration_t 	m_WifiConfig;
	ApConfiguration_t		m_ApConfig;

	void run(void* data);

	void enableEthernet(void);
	void disableEthernet(void);

	void enableWifiStation(std::string ssid, std::string passcode);
	void disableWifiStation(void);

	void enableWifiAccessPoint(std::string ssid, std::string passcode);
	void disableWifiAccessPoint(void);

	void handleClients();

};

#endif