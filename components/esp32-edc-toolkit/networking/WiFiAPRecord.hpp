/**
 * @brief
 *
 * @file WiFiAPRecord.hpp
 * @author your name
 * @date 2018-07-11
 */

#ifndef WIFI_APRECORD_H_
#define WIFI_APRECORD_H_

#include "esp_wifi.h"
class WiFiAPRecord
{
  public:
	WiFiAPRecord(){

	}

	friend class WiFi;

	/**
     * @brief Get the auth mode.
     * @return The auth mode.
     */
	wifi_auth_mode_t getAuthMode()
	{
		return m_authMode;
	}

	/**
     * @brief Get the RSSI.
     * @return the RSSI.
     */
	int8_t getRSSI()
	{
		return m_rssi;
	}

	/**
     * @brief Get the SSID.
     * @return the SSID.
     */
	std::string getSSID()
	{
		return m_ssid;
	}

	std::string toString();

  private:
	uint8_t m_bssid[6];
	int8_t m_rssi;
	std::string m_ssid;
	wifi_auth_mode_t m_authMode;
};

#endif