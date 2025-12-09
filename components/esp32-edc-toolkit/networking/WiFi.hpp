/**
 * @brief
 *
 * @file WiFi.hpp
 * @author your name
 * @date 2018-07-11
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include <string>
#include <vector>
#include <esp_err.h>
#include "FreeRTOS.hpp"

#include "WiFiAPRecord.hpp"

class WiFi
{
  public:
	WiFi();
	~WiFi();

	bool startAP(const std::string &ssid, const std::string &passwd, wifi_auth_mode_t auth, uint8_t channel, bool ssid_hidden, uint8_t max_connection);
	bool startAP(const std::string &ssid, const std::string &passwd, wifi_auth_mode_t auth = WIFI_AUTH_OPEN);
	bool stopAP(void);

	bool startSTA(const std::string &ssid, const std::string &password);
	bool stopSTA(void);

  private:
	void init();
	bool setMode(wifi_mode_t newMode);
	wifi_mode_t getMode(void);

	wifi_mode_t m_WiFiMode{WIFI_MODE_NULL};	//Current WiFi mode, evaluated to switch dynamically
	bool m_initCalled{false};				//WiFi Stack initialized?
	bool m_enabled{false};					//WiFi enabled?
	bool m_apConnected; 					// Are we connected to an access point?

	uint32_t ip;
	uint32_t gw;
	uint32_t netmask;

	uint8_t m_dnsCount = 0;


	FreeRTOS::Semaphore m_connectFinished = FreeRTOS::Semaphore("ConnectFinished");

  public:

	std::string getApMac();
	tcpip_adapter_ip_info_t getApIpInfo();
	std::string getApSSID();
	std::string getApIp();
	std::string getApNetmask();
	std::string getApGateway();

	tcpip_adapter_ip_info_t getStaIpInfo();
	std::string getStaMac();
	std::string getStaSSID();
	std::string getStaIp();
	std::string getStaNetmask();
	std::string getStaGateway();
	std::vector<WiFiAPRecord> scan();
	void setIPInfo(const std::string &ip, const std::string &gw, const std::string &netmask);
	void setIPInfo(const char *ip, const char *gw, const char *netmask);
	void setIPInfo(uint32_t ip, uint32_t gw, uint32_t netmask);
	//void                      setWifiEventHandler(WiFiEventHandler *wifiEventHandler);
	void stop(void);
};

#endif /* MAIN_WIFI_H_ */
