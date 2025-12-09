/*
 * NetworkEventHandler.cpp
 *
 *  Created on: 11.07.2017
 *      Author: kolban, marcel.seerig
 */

#include "NetworkEventHandler.hpp"
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <esp_log.h>
#include "sdkconfig.h"

static const char* LOG_TAG = "NetworkEventHandler";

void NetworkEventHandler::ethernetEventHandler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
	NetworkEventHandler *m_handler = reinterpret_cast<NetworkEventHandler*>(arg);
	if (m_handler == nullptr) {
		ESP_LOGE(LOG_TAG, "No context");
		abort();
	}

	ESP_LOGD(LOG_TAG, "ethernetEventHandler called with ID [%d]", event_id);

	switch (event_id) {

		case ETHERNET_EVENT_START: {
			m_handler->ethStart();
			break;
		}
		case ETHERNET_EVENT_STOP: {
			m_handler->ethStop();
			break;
		}
		case ETHERNET_EVENT_CONNECTED: {
			m_handler->ethConnected();
			break;
		}
		case ETHERNET_EVENT_DISCONNECTED: {
			m_handler->ethDisconnected();
			break;
		}

		default:
			break;
	}

} // ethernetEventHandler

void NetworkEventHandler::wifiEventHandler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
	NetworkEventHandler *m_handler = reinterpret_cast<NetworkEventHandler*>(arg);
	if (m_handler == nullptr) {
		ESP_LOGE(LOG_TAG, "No context");
		abort();
	}

	ESP_LOGD(LOG_TAG, "wifiEventHandler called with ID [%d]", event_id);

	switch(event_id) {
		// WIFI STA
		case WIFI_EVENT_WIFI_READY: {
			m_handler->wifiReady();
			break;
		}
		case WIFI_EVENT_SCAN_DONE: {
			m_handler->staScanDone();
			break;
		}
		case WIFI_EVENT_STA_START: {
			m_handler->staStart();
			break;
		}
		case WIFI_EVENT_STA_STOP: {
			m_handler->staStop();
			break;
		}
		case WIFI_EVENT_STA_CONNECTED: {
			m_handler->staConnected();
			break;
		}
		case WIFI_EVENT_STA_DISCONNECTED: {
			m_handler->staDisconnected();
			break;
		}
		case WIFI_EVENT_STA_AUTHMODE_CHANGE: {
			m_handler->staAuthChange();
			break;
		}
		// WPS
		case WIFI_EVENT_STA_WPS_ER_SUCCESS: {
			m_handler->staWpsSuccess();
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_FAILED: {
			m_handler->staWpsFailed();
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_TIMEOUT: {
			m_handler->staWpsTimeout();
			break;
		}
		case WIFI_EVENT_STA_WPS_ER_PIN: {
			m_handler->staWpsPin();
			break;
		}
		// WIFI AP
		case WIFI_EVENT_AP_START: {
			m_handler->apStart();
			break;
		}
		case WIFI_EVENT_AP_STOP: {
			m_handler->apStop();
			break;
		}
		case WIFI_EVENT_AP_STACONNECTED: {
			m_handler->apStaConnected();
			break;
		}
		case WIFI_EVENT_AP_STADISCONNECTED: {
			m_handler->apStaDisconnected();
			break;
		}
		case WIFI_EVENT_AP_PROBEREQRECVED: {
			m_handler->apProbeRequestReceived();
			break;
		}

		default:
			break;
    }

} // wifiEventHandler


void NetworkEventHandler::ipEventHandler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
	NetworkEventHandler *m_handler = reinterpret_cast<NetworkEventHandler*>(arg);
	if (m_handler == nullptr) {
		ESP_LOGE(LOG_TAG, "No context");
		abort();
	}

	ESP_LOGD(LOG_TAG, "ipEventHandler called with ID [%d]", event_id);

	switch(event_id) {
		// WIFI STA
		case IP_EVENT_STA_GOT_IP: {
			m_handler->staGotIp();
			break;
		}
		case IP_EVENT_STA_LOST_IP: {
			m_handler->staLostIp();
			break;
		}
		case IP_EVENT_AP_STAIPASSIGNED: {
			m_handler->apStaIpAssigned();
			break;
		}
		case IP_EVENT_GOT_IP6: {
			m_handler->gotIP6();
			break;
		}
		case IP_EVENT_ETH_GOT_IP: {
			ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
			esp_netif_ip_info_t *ip_info = &event->ip_info;
			m_handler->ethGotIp(ip_info);
			break;
		}
		case IP_EVENT_ETH_LOST_IP: {
			m_handler->ethLostIp();
			break;
		}

		default:
			break;
    }

} // ipEventHandler


/**
 * @brief Constructor
 */
NetworkEventHandler::NetworkEventHandler(void *data) {
	m_data = data;
	esp_event_handler_t eth_handler = NetworkEventHandler::ethernetEventHandler;
	esp_event_handler_t wifi_handler = NetworkEventHandler::wifiEventHandler;
	esp_event_handler_t ip_handler = NetworkEventHandler::ipEventHandler;
	ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_handler, this));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler, this));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, ip_handler, this));
} // NetworkEventHandler

esp_err_t NetworkEventHandler::ethStart() {
	ESP_LOGD(LOG_TAG, "default ethStart");
	return ESP_OK;
} // ethStart

esp_err_t NetworkEventHandler::ethStop() {
	ESP_LOGD(LOG_TAG, "default ethStop");
	return ESP_OK;
} // ethStop

esp_err_t NetworkEventHandler::ethConnected() {
	ESP_LOGD(LOG_TAG, "default ethConnected");
	return ESP_OK;
} // ethConnected

esp_err_t NetworkEventHandler::ethDisconnected() {
	ESP_LOGD(LOG_TAG, "default ethDisconnected");
	return ESP_OK;
} // ethDisconnected

esp_err_t NetworkEventHandler::wifiReady() {
    ESP_LOGD(LOG_TAG, "default wifiReady");
    return ESP_OK;
} // wifiReady

esp_err_t NetworkEventHandler::staScanDone() {
    ESP_LOGD(LOG_TAG, "default staScanDone");
    return ESP_OK;
} // staScanDone

esp_err_t NetworkEventHandler::staStart() {
    ESP_LOGD(LOG_TAG, "default staStart");
    return ESP_OK;
} // staStart

esp_err_t NetworkEventHandler::staStop() {
    ESP_LOGD(LOG_TAG, "default staStop");
    return ESP_OK;
} // staStop

esp_err_t NetworkEventHandler::staConnected() {
    ESP_LOGD(LOG_TAG, "default staConnected");
    return ESP_OK;
} // staConnected

esp_err_t NetworkEventHandler::staDisconnected() {
    ESP_LOGD(LOG_TAG, "default staDisconnected");
    return ESP_OK;
} // staDisconnected

esp_err_t NetworkEventHandler::staAuthChange() {
    ESP_LOGD(LOG_TAG, "default staAuthChange");
    return ESP_OK;
} // staAuthChange

esp_err_t NetworkEventHandler::staWpsSuccess() {
	ESP_LOGD(LOG_TAG, "default staWpsSuccess");
	return ESP_OK;
} // staWpsSuccess

/**
 * @brief Handle the wps fails in enrollee event.
 * Handle having wps fails in enrollee mode.
 * @param [in] sta_er_fail_reason system_event_sta_wps_fail_reason_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::staWpsFailed() {
	ESP_LOGD(LOG_TAG, "default staWpsFailed");
	return ESP_OK;
} // staWpsFailed

/**
 * @brief Handle the wps timeout in enrollee event.
 * Handle having wps timeout in enrollee mode.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::staWpsTimeout() {
	ESP_LOGD(LOG_TAG, "default staWpsTimeout");
	return ESP_OK;
} // staWpsTimeout

/**
 * @brief Handle the wps pin code in enrollee event.
 * Handle having wps pin code in enrollee mode.
 * @param [in] sta_er_pin system_event_sta_wps_er_pin_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::staWpsPin() {
	ESP_LOGD(LOG_TAG, "default staWpsPin");
	return ESP_OK;
} // staWpsPin

/**
 * @brief Handle the Access Point started event.
 * Handle an indication that the ESP32 has started being an access point.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::apStart() {
    ESP_LOGD(LOG_TAG, "default apStart");
    return ESP_OK;
} // apStart

/**
 * @brief Handle the Access Point stop event.
 * Handle an indication that the ESP32 has stopped being an access point.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::apStop() {
    ESP_LOGD(LOG_TAG, "default apStop");
    return ESP_OK;
} // apStop

/**
 * @brief Handle a Station Connected to AP event.
 * Handle having a station connected to ESP32 soft-AP.
 * @param [in] event_sta_connected system_event_ap_staconnected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::apStaConnected() {
    ESP_LOGD(LOG_TAG, "default apStaConnected");
    return ESP_OK;
} // apStaConnected

/**
 * @brief Handle a Station Disconnected from AP event.
 * Handle having a station disconnected from ESP32 soft-AP.
 * @param [in] event_sta_disconnected system_event_ap_stadisconnected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::apStaDisconnected() {
    ESP_LOGD(LOG_TAG, "default apStaDisconnected");
    return ESP_OK;
} // apStaDisconnected

/**
 * @brief Handle the soft-AP receive probe request packet event.
 * Handle having received a probe request packet.
 * @param [in] ap_probereqrecved system_event_ap_probe_req_rx_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::apProbeRequestReceived() {
	ESP_LOGD(LOG_TAG, "default apProbeRequestReceived");
	return ESP_OK;
} // apProbeRequestReceived

/**
 * @brief Handle the Station Got IP event.
 * Handle having received/assigned an IP address when we are a station.
 * @param [in] event_sta_got_ip The Station Got IP event.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::staGotIp() {
    ESP_LOGD(LOG_TAG, "default staGotIp");
    return ESP_OK;
} // staGotIp

/**
 * @brief Handle the Station Lost IP event.
 * Handle having received/assigned an IP address drop when we are a station.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::staLostIp() {
	ESP_LOGD(LOG_TAG, "default staLostIp");
	return ESP_OK;
} // staLostIp

esp_err_t NetworkEventHandler::apStaIpAssigned() {
	ESP_LOGD(LOG_TAG, "default apStaIpAssigned");
	return ESP_OK;
} // apStaIpAssigned

/**
 * @brief Handle the gotIP6 event.
 * Handle having the auth mode of AP ESP32 station connected to changed.
 * @param [in] got_ip6 system_event_got_ip6_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t NetworkEventHandler::gotIP6() {
	ESP_LOGD(LOG_TAG, "default gotIP6");
	return ESP_OK;
} // gotIP6

esp_err_t NetworkEventHandler::ethGotIp(esp_netif_ip_info_t * ip_info) {
	ESP_LOGD(LOG_TAG, "default ethGotIp");
	return ESP_OK;
} // ethGotIp

esp_err_t NetworkEventHandler::ethLostIp(){
	ESP_LOGD(LOG_TAG, "default ethLostIp");
	return ESP_OK;
} // ethLostIp


NetworkEventHandler::~NetworkEventHandler() {
} // ~NetworkEventHandler
