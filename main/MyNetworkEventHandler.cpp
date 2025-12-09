/**
 * @brief
 *
 * @file MyNetworkEventHandler.cpp
 * @author your name
 * @date 2018-07-11
 */

#include "MyNetworkEventHandler.hpp"

#include "esp_log.h"
#include "esp_netif_ip_addr.h"
#include <sstream>

static const char* LOGTAG = "MyNetworkEventHandler";

esp_err_t MyNetworkEventHandler::ethGotIp(esp_netif_ip_info_t *ip_info) {
    ESP_LOGI(LOGTAG, "ETH got IP");
    m_EthHasIP = true;

    // Convert IP address to string format
    char ip_str[IP4ADDR_STRLEN_MAX];
    esp_ip4addr_ntoa(&ip_info->ip, ip_str, IP4ADDR_STRLEN_MAX);
    m_EthernetIP = std::string(ip_str);

    // Convert gateway address to string format
    char gw_str[IP4ADDR_STRLEN_MAX];
    esp_ip4addr_ntoa(&ip_info->gw, gw_str, IP4ADDR_STRLEN_MAX);
    m_EthernetGateway = std::string(gw_str);

    return ESP_OK;
}

esp_err_t MyNetworkEventHandler::ethLostIp(){
	ESP_LOGI(LOGTAG, "ETH lost ip");
	m_EthHasIP = false;
	m_EthernetIP = "";
	m_EthernetGateway = "";
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::ethStart() {
	ESP_LOGI(LOGTAG, "ETH start");
	m_EthIsStarted = true;
	return ESP_OK;
} // ethStart

esp_err_t MyNetworkEventHandler::ethStop() {
	ESP_LOGI(LOGTAG, "ETH stop");
	m_EthIsStarted = false;
	m_EthIsConnected = false;
	m_EthHasIP = false;
	m_EthernetIP = "0.0.0.0";
	m_EthernetGateway = "0.0.0.0";
	return ESP_OK;
} // ethStop

esp_err_t MyNetworkEventHandler::ethConnected() {
	ESP_LOGI(LOGTAG, "ETH connected");
	m_EthIsConnected = true;
	return ESP_OK;
} // ethConnected

esp_err_t MyNetworkEventHandler::ethDisconnected() {
	ESP_LOGI(LOGTAG, "ETH disconnected");
	m_EthIsConnected = false;
	m_EthHasIP = false;
	m_EthernetIP = "0.0.0.0";
	m_EthernetGateway = "0.0.0.0";
	return ESP_OK;
} // ethDisconnected

esp_err_t MyNetworkEventHandler::apStaConnected()
{
	ESP_LOGI(LOGTAG, "AP Station connected");
	m_APNumberOfStations++;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::apStaDisconnected()
{
	ESP_LOGI(LOGTAG, "AP Station disconnected");
	if (m_APNumberOfStations > 0)
		m_APNumberOfStations--;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::apStart()
{
	ESP_LOGI(LOGTAG, "AP started");
	m_APstarted = true;
	m_APNumberOfStations = 0;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::apStop()
{
	ESP_LOGI(LOGTAG, "AP stopped");
	m_APstarted = false;
	m_APNumberOfStations = 0;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staConnected() {
	ESP_LOGI(LOGTAG, "STA connected");
	m_StaIsConnected = true;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staDisconnected() {
	ESP_LOGI(LOGTAG, "STA disconnected");
	m_StaIsConnected = false;
	m_StaHasIP = false;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staGotIp() {
	ESP_LOGI(LOGTAG, "STA got IP");
	m_StaHasIP = true;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staLostIp() {
	ESP_LOGI(LOGTAG, "STA lost IP");
	m_StaHasIP = false;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staStart(){
	ESP_LOGI(LOGTAG, "STA started");
	m_StaIsStarted = true;
	return ESP_OK;
}

esp_err_t MyNetworkEventHandler::staStop(){
	ESP_LOGI(LOGTAG, "STA stopped");
	m_StaIsStarted = false;
	m_StaIsConnected = false;
	m_StaHasIP = false;
	return ESP_OK;
}
