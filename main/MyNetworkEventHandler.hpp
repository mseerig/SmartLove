/**
 * @file MyNetworkEventHandler.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2021-01-06
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef MY_NETWORK_ETHERNET_EVENT_HANDLER_H_
#define MY_NETWORK_ETHERNET_EVENT_HANDLER_H_

#include "NetworkEventHandler.hpp"
#include <string>

class MyNetworkEventHandler : public NetworkEventHandler {
public:

	esp_err_t ethStart();
	esp_err_t ethStop();
	esp_err_t ethConnected();
	esp_err_t ethDisconnected();

	// esp_err_t wifiReady();
	// esp_err_t staScanDone();
	esp_err_t staStart();
	esp_err_t staStop();
	esp_err_t staConnected();
	esp_err_t staDisconnected();
	// esp_err_t staAuthChange();

	// esp_err_t staWpsSuccess();
	// esp_err_t staWpsFailed();
	// esp_err_t staWpsTimeout();
	// esp_err_t staWpsPin();

	esp_err_t apStart();
	esp_err_t apStop();
	esp_err_t apStaConnected();
	esp_err_t apStaDisconnected();

	// esp_err_t apProbeRequestReceived();

	esp_err_t staGotIp();
	esp_err_t staLostIp();
	// esp_err_t apStaIpAssigned();
	esp_err_t ethGotIp(esp_netif_ip_info_t * ip_info);
	esp_err_t ethLostIp();
	// esp_err_t gotIP6();

	bool isStationConnected(){
		if (m_APNumberOfStations > 0)
		{
			return true;
		}
		return false;
	}
	bool isAPstarted(){return m_APstarted;}

	bool isStaStarted(void) {return m_StaIsStarted;}
	bool isStaConnected(void) {return m_StaIsConnected;}
	bool hasStaIP(void) {return m_StaHasIP;}

	bool isEthStarted(void) {return m_EthIsStarted;}
	bool isEthConnected(void) {return m_EthIsConnected;}
	bool hasEthIP(void) {return m_EthHasIP;}

	std::string getEthernetIP(){return m_EthernetIP;}
	std::string getEthernetGateway(){return m_EthernetGateway;}

private:

	bool 	m_APstarted{false};
	uint8_t m_APNumberOfStations{0};

	bool 	m_StaIsStarted{false};
	bool	m_StaIsConnected{false};
	bool	m_StaHasIP{false};

	bool 	m_EthIsStarted{false};
	bool 	m_EthIsConnected{false};
	bool 	m_EthHasIP{false};

	std::string m_EthernetIP = {"0.0.0.0"};
	std::string m_EthernetGateway = {"0.0.0.0"};
};

#endif