/*
 * WiFi.cpp
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

//#define _GLIBCXX_USE_C99
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "sdkconfig.h"
#include "WiFi.hpp"
#include "GeneralUtils.hpp"
#include <freertos/FreeRTOS.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include "lwip/err.h"
#include "lwip/sys.h"


static const char *LOG_TAG = "WiFi";

/**
 * @brief Creates and uses a default event handler
 */
WiFi::WiFi()
	: ip(0), gw(0), netmask(0)
{
	m_initCalled = false;
	m_apConnected = false; // Are we connected to an access point?
	esp_log_level_set("phy_init", ESP_LOG_NONE);
} // WiFi

/**
 * @brief Deletes the event handler that was used by the class
 */
WiFi::~WiFi()
{

} // ~WiFi

/**
  * @brief  Stop WiFi
  * @param N/A.
  * @return N/A.
  */
void WiFi::stop(void){
	if(m_initCalled) ESP_ERROR_CHECK(esp_wifi_stop());
} // stop

/**
 * @brief Get the AP IP Info.
 * @return The AP IP Info.
 */
tcpip_adapter_ip_info_t WiFi::getApIpInfo(){
	init();
	tcpip_adapter_ip_info_t ipInfo;
	::tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
	return ipInfo;
} // getApIpInfo

/**
 * @brief Get the MAC address of the AP interface.
 * @return The MAC address of the AP interface.
 */
std::string WiFi::getApMac(){
	uint8_t mac[6];
	init();
	esp_wifi_get_mac(WIFI_IF_AP, mac);
	auto mac_str = (char *)malloc(18);
	sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return std::string(std::move(mac_str));
} // getApMac

/**
 * @brief Get the AP SSID.
 * @return The AP SSID.
 */
std::string WiFi::getApSSID(){
	wifi_config_t conf = wifi_config_t();
	init();
	esp_wifi_get_config(WIFI_IF_AP, &conf);
	return std::string((char *)conf.sta.ssid);
} // getApSSID

/**
 * @brief Get the current ESP32 IP form AP.
 * @return The ESP32 IP.
 */
std::string WiFi::getApIp(){
	tcpip_adapter_ip_info_t ipInfo = getApIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.ip.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaIp

/**
 * @brief Get the current AP netmask.
 * @return The Netmask IP.
 */
std::string WiFi::getApNetmask(){
	tcpip_adapter_ip_info_t ipInfo = getApIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.netmask.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaNetmask

/**
 * @brief Get the current AP Gateway IP.
 * @return The Gateway IP.
 */
std::string WiFi::getApGateway(){
	tcpip_adapter_ip_info_t ipInfo = getApIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.gw.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaGateway

/**
 * @brief Get the STA IP Info.
 * @return The STA IP Info.
 */
tcpip_adapter_ip_info_t WiFi::getStaIpInfo()
{
	tcpip_adapter_ip_info_t ipInfo;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	return ipInfo;
} // getStaIpInfo

/**
 * @brief Get the current ESP32 IP form STA.
 * @return The ESP32 IP.
 */
std::string WiFi::getStaIp()
{
	tcpip_adapter_ip_info_t ipInfo = getStaIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.ip.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaIp

/**
 * @brief Get the current STA netmask.
 * @return The Netmask IP.
 */
std::string WiFi::getStaNetmask()
{
	tcpip_adapter_ip_info_t ipInfo = getStaIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.netmask.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaNetmask

/**
 * @brief Get the current STA Gateway IP.
 * @return The Gateway IP.
 */
std::string WiFi::getStaGateway()
{
	tcpip_adapter_ip_info_t ipInfo = getStaIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.gw.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getStaGateway

/**
 * @brief Get the MAC address of the STA interface.
 * @return The MAC address of the STA interface.
 */
std::string WiFi::getStaMac()
{
	uint8_t mac[6];
	esp_wifi_get_mac(WIFI_IF_STA, mac);
	auto mac_str = (char *)malloc(18);
	sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return std::string(std::move(mac_str));
} // getStaMac

/**
 * @brief Get the STA SSID.
 * @return The STA SSID.
 */
std::string WiFi::getStaSSID()
{
	wifi_config_t conf = wifi_config_t();
	esp_wifi_get_config(WIFI_IF_STA, &conf);
	return std::string((char *)conf.ap.ssid);
} // getStaSSID

/**
 * @brief Initialize WiFi.
 */
void WiFi::init()
{
	if (!m_initCalled){
		::esp_netif_create_default_wifi_ap();
		::esp_netif_create_default_wifi_sta();

		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		esp_err_t errRc = ::esp_wifi_init(&cfg);
		if (errRc != ESP_OK)
		{
			ESP_LOGE(LOG_TAG, "esp_wifi_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			abort();
		}

		/*errRc = ::esp_wifi_set_storage(WIFI_STORAGE_RAM);
		if (errRc != ESP_OK)
		{
			ESP_LOGE(LOG_TAG, "esp_wifi_set_storage: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			abort();
		}*/
	}
	m_initCalled = true;
}

bool WiFi::setMode(wifi_mode_t newMode)
{
	wifi_mode_t currentMode = getMode();

	if (currentMode == newMode)
	{ //Needed mode is already set
		return true;
	}
	else
	{ //New mode needs to be set
		esp_err_t err;
		err = esp_wifi_set_mode(newMode);
		if (err)
		{
			ESP_LOGE(LOG_TAG, "Could not set WiFi mode! %u", err);
			return false;
		}
		ESP_LOGI(LOG_TAG, "WiFi mode set %u", newMode);
		err = esp_wifi_get_mode(&m_WiFiMode);
	}
	return true;
}

wifi_mode_t WiFi::getMode(void)
{
	return m_WiFiMode;
}

/**
 * @brief Perform a WiFi scan looking for access points.
 *
 * An access point scan is performed and a vector of WiFi access point records
 * is built and returned with one record per found scan instance.  The scan is
 * performed in a blocking fashion and will not return until the set of scanned
 * access points has been built.
 *
 * @return A vector of WiFiAPRecord instances.
 */
std::vector<WiFiAPRecord> WiFi::scan()
{
	ESP_LOGD(LOG_TAG, ">> scan");
	std::vector<WiFiAPRecord> apRecords;

	init();

	esp_err_t errRc = ::esp_wifi_set_mode(WIFI_MODE_STA);
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_set_mode: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}

	errRc = ::esp_wifi_start();
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}

	wifi_scan_config_t conf;
	memset(&conf, 0, sizeof(conf));
	conf.show_hidden = true;

	esp_err_t rc = ::esp_wifi_scan_start(&conf, true);
	if (rc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_scan_start: %d", rc);
		return apRecords;
	}

	uint16_t apCount; // Number of access points available.
	rc = ::esp_wifi_scan_get_ap_num(&apCount);
	ESP_LOGD(LOG_TAG, "Count of found access points: %d", apCount);

	wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
	if (list == nullptr)
	{
		ESP_LOGE(LOG_TAG, "Failed to allocate memory");
		return apRecords;
	}

	errRc = ::esp_wifi_scan_get_ap_records(&apCount, list);
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_scan_get_ap_records: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}

	for (auto i = 0; i < apCount; i++)
	{
		WiFiAPRecord wifiAPRecord;
		memcpy(wifiAPRecord.m_bssid, list[i].bssid, 6);
		wifiAPRecord.m_ssid = std::string((char *)list[i].ssid);
		wifiAPRecord.m_authMode = list[i].authmode;
		wifiAPRecord.m_rssi = list[i].rssi;
		apRecords.push_back(wifiAPRecord);
	}
	free(list); // Release the storage allocated to hold the records.
	std::sort(apRecords.begin(),
			  apRecords.end(),
			  [](const WiFiAPRecord &lhs, const WiFiAPRecord &rhs) { return lhs.m_rssi > rhs.m_rssi; });
	return apRecords;
} // scan

bool WiFi::startSTA(const std::string &ssid, const std::string &password)
{

	ESP_LOGI(LOG_TAG, ">> startSTA: ssid: %s", ssid.c_str());
	init();

	wifi_mode_t currentMode = getMode();
	bool isEnabled = ((currentMode & WIFI_MODE_STA) != 0);

	if (!isEnabled)
	{
		if (!setMode((wifi_mode_t)(currentMode | WIFI_MODE_STA)))
		{
			ESP_LOGE(LOG_TAG, ">> setMode failed");
			return false;
		}
	}

	wifi_config_t sta_config = wifi_config_t();

	::memset(&sta_config, 0, sizeof(sta_config));
	::memcpy(sta_config.sta.ssid, ssid.data(), ssid.size());
	::memcpy(sta_config.sta.password, password.data(), password.size());
	sta_config.sta.bssid_set = 0;

	esp_err_t errRc;
	errRc = ::esp_wifi_set_config(WIFI_IF_STA, &sta_config);
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_set_config: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	errRc = ::esp_wifi_start();
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	ESP_LOGI(LOG_TAG, "esp_wifi_connect");
	errRc = ::esp_wifi_connect();
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_connect: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	/*errRc = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);

	if ((errRc != ESP_OK) && (errRc != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED))
	{
		ESP_LOGE(LOG_TAG, "tcpip_adapter_dhcpc_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}
*/
	return true;
}

bool WiFi::stopSTA(void)
{
	esp_err_t errRc;

	ESP_LOGI(LOG_TAG, ">> stopSTA");

	/*errRc = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); //Stop dhcp server
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "tcpip_adapter_dhcpc_stop: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}*/

	errRc = esp_wifi_disconnect();
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_disconnect: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	wifi_mode_t currentMode = getMode();
	bool isEnabled = ((currentMode & WIFI_MODE_STA) != 0);

	if (isEnabled)
	{
		if (!setMode((wifi_mode_t)(currentMode & (~WIFI_MODE_STA))))
		{
			ESP_LOGE(LOG_TAG, ">> setMode failed");
			return false;
		}
	}

	return true;
}

/**
 * @brief Start being an access point.
 *
 * @param[in] ssid The SSID to use to advertize for stations.
 * @param[in] password The password to use for station connections.
 * @param[in] auth The authorization mode for access to this access point.  Options are:
 * * WIFI_AUTH_OPEN
 * * WIFI_AUTH_WPA_PSK
 * * WIFI_AUTH_WPA2_PSK
 * * WIFI_AUTH_WPA_WPA2_PSK
 * * WIFI_AUTH_WPA2_ENTERPRISE
 * * WIFI_AUTH_WEP
 * @return N/A.
 */
bool WiFi::startAP(const std::string &ssid, const std::string &password, wifi_auth_mode_t auth)
{
	return startAP(ssid, password, auth, 6, false, 4);
} // startAP

/**
 * @brief Start being an access point.
 *
 * @param[in] ssid The SSID to use to advertize for stations.
 * @param[in] password The password to use for station connections.
 * @param[in] auth The authorization mode for access to this access point.  Options are:
 * * WIFI_AUTH_OPEN
 * * WIFI_AUTH_WPA_PSK
 * * WIFI_AUTH_WPA2_PSK
 * * WIFI_AUTH_WPA_WPA2_PSK
 * * WIFI_AUTH_WPA2_ENTERPRISE
 * * WIFI_AUTH_WEP
 * @param[in] channel from the access point.
 * @param[in] is the ssid hidden, ore not.
 * @param[in] limiting number of clients.
 * @return N/A.
 */
bool WiFi::startAP(const std::string &ssid, const std::string &password, wifi_auth_mode_t auth, uint8_t channel, bool ssid_hidden, uint8_t max_connection)
{
	esp_err_t errRc;

	ESP_LOGD(LOG_TAG, ">> startAP: ssid: %s", ssid.c_str());

	init();

	wifi_mode_t currentMode = getMode();
	bool isEnabled = ((currentMode & WIFI_MODE_AP) != 0);

	if (!isEnabled)
	{
		if (!setMode((wifi_mode_t)(currentMode | WIFI_MODE_AP)))
		{
			ESP_LOGE(LOG_TAG, ">> setMode failed");
			return false;
		}
	}

	// Build the apConfig structure.
	wifi_config_t apConfig = wifi_config_t();
	::memset(&apConfig, 0, sizeof(apConfig));
	::memcpy(apConfig.ap.ssid, ssid.data(), ssid.size());
	apConfig.ap.ssid_len = ssid.size();
	::memcpy(apConfig.ap.password, password.data(), password.size());
	apConfig.ap.channel = channel;
	apConfig.ap.authmode = auth;
	apConfig.ap.ssid_hidden = (uint8_t)ssid_hidden;
	apConfig.ap.max_connection = max_connection;
	apConfig.ap.beacon_interval = 100;

	errRc = ::esp_wifi_set_config(WIFI_IF_AP, &apConfig);
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_set_config: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	errRc = ::esp_wifi_start();
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "esp_wifi_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}

	/*errRc = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);

	if ((errRc != ESP_OK) && (errRc != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED))
	{
		ESP_LOGE(LOG_TAG, "tcpip_adapter_dhcps_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}*/

	ESP_LOGI(LOG_TAG, "<< startAP");

	return true;
} // startAP

bool WiFi::stopAP(void)
{
	ESP_LOGI(LOG_TAG, ">> stopAP");

	/*esp_err_t errRc;

	errRc = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP); //Stop dhcp server
	if (errRc != ESP_OK)
	{
		ESP_LOGE(LOG_TAG, "tcpip_adapter_dhcps_stop: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return false;
	}
*/
	wifi_mode_t currentMode = getMode();
	bool isEnabled = ((currentMode & WIFI_MODE_AP) != 0);

	if (isEnabled)
	{
		if (!setMode((wifi_mode_t)(currentMode & (~WIFI_MODE_AP))))
		{
			ESP_LOGE(LOG_TAG, ">> setMode failed");
			return false;
		}
	}
	ESP_LOGI(LOG_TAG, "<< stopAP");
	return true;
}

/**
 * @brief Set the IP info and enable DHCP if ip != 0. If called with ip == 0 then DHCP is enabled.
 * If called with bad values it will do nothing.
 *
 * Do not call this method if we are being an access point ourselves.
 *
 * For example, prior to calling `connectAP()` we could invoke:
 *
 * @code{.cpp}
 * myWifi.setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
 * @endcode
 *
 * @param [in] ip IP address value.
 * @param [in] gw Gateway value.
 * @param [in] netmask Netmask value.
 * @return N/A.
 */
void WiFi::setIPInfo(const std::string &ip, const std::string &gw, const std::string &netmask)
{
	setIPInfo(ip.c_str(), gw.c_str(), netmask.c_str());
} // setIPInfo

void WiFi::setIPInfo(const char *ip, const char *gw, const char *netmask)
{
	uint32_t new_ip=0;
	uint32_t new_gw=0;
	uint32_t new_netmask=0;

	auto success = (bool)inet_pton(AF_INET, ip, &new_ip);
	success = success && inet_pton(AF_INET, gw, &new_gw);
	success = success && inet_pton(AF_INET, netmask, &new_netmask);

	if (!success)
	{
		return;
	}

	setIPInfo(new_ip, new_gw, new_netmask);
} // setIPInfo

/**
 * @brief Set the IP Info based on the IP address, gateway and netmask.
 * @param [in] ip The IP address of our ESP32.
 * @param [in] gw The gateway we should use.
 * @param [in] netmask Our TCP/IP netmask value.
 */
void WiFi::setIPInfo(uint32_t ip, uint32_t gw, uint32_t netmask)
{
	init();

	this->ip = ip;
	this->gw = gw;
	this->netmask = netmask;

	if (ip != 0 && gw != 0 && netmask != 0)
	{
		tcpip_adapter_ip_info_t ipInfo;
		ipInfo.ip.addr = ip;
		ipInfo.gw.addr = gw;
		ipInfo.netmask.addr = netmask;
		::tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
		::tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	}
	else
	{
		ip = 0;
		::tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
	}
} // setIPInfo

/**
 * @brief Return a string representation of the WiFi access point record.
 *
 * @return A string representation of the WiFi access point record.
 */
std::string WiFiAPRecord::toString()
{
	std::string auth;
	switch (getAuthMode())
	{
	case WIFI_AUTH_OPEN:
		auth = "WIFI_AUTH_OPEN";
		break;
	case WIFI_AUTH_WEP:
		auth = "WIFI_AUTH_WEP";
		break;
	case WIFI_AUTH_WPA_PSK:
		auth = "WIFI_AUTH_WPA_PSK";
		break;
	case WIFI_AUTH_WPA2_PSK:
		auth = "WIFI_AUTH_WPA2_PSK";
		break;
	case WIFI_AUTH_WPA_WPA2_PSK:
		auth = "WIFI_AUTH_WPA_WPA2_PSK";
		break;
	default:
		auth = "<unknown>";
		break;
	}
	//    std::stringstream s;
	//    s<< "ssid: " << m_ssid << ", auth: " << auth << ", rssi: " << m_rssi;
	auto info_str = (char *)malloc(6 + 32 + 8 + 22 + 8 + 3 + 1);
	sprintf(info_str, "ssid: %s, auth: %s, rssi: %d", m_ssid.c_str(), auth.c_str(), (int)m_rssi);
	return std::string(std::move(info_str));
} // toString