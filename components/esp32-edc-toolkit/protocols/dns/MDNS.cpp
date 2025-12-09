/**
 * @file MDNS.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-03-15
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "MDNS.hpp"

#include <string>
#include "mdns.h"
#include "esp_log.h"

static const char *TAG = "mDNS";

/**
 * @brief MDNS init.
 * @param  hostname     	Hostname to set
 * @param  instance_name    Instance name to set
 */
MDNS::MDNS(std::string hostname, std::string instanceName):
	m_hostname(hostname),
	m_instanceName(instanceName)
	{

	if(mdns_init() == ESP_OK){
		setHostname(m_hostname);
		if(instanceName != "") setInstanceName(m_instanceName);
		ESP_LOGD(TAG, "set hostname '%s', instance name '%s'", m_hostname.c_str(), m_instanceName.c_str());
	}
}

/**
 * @brief MDNS destructor.
 */
MDNS::~MDNS(){
	if(mdns_service_remove_all() == ESP_OK)	mdns_free();
}

/**
 * @brief refresh the mDNS for new connections
 *
 */
void MDNS::refresh(void){
	if(mdns_service_remove_all() == ESP_OK)	mdns_free();
	if(mdns_init() == ESP_OK){
		setHostname(m_hostname);
		if(m_instanceName != "") setInstanceName(m_instanceName);
		ESP_LOGD(TAG, "refresh");
	}
}

/**
 * @brief  Set the hostname for mDNS server
 *         required if you want to advertise services
 * @param  hostname     Hostname to set
 * @return N/A.
 */
void MDNS::setHostname(std::string hostname){
	if(mdns_hostname_set(hostname.c_str()) != ESP_OK)
		ESP_LOGE(TAG, "Error while setting hostname");
}

/**
 * @brief  Set the default instance name for mDNS server
 * @param  instance_name     Instance name to set
 * @return N/A.
 */
void MDNS::setInstanceName(std::string instanceName){
	m_instanceName = instanceName;
	if(mdns_instance_name_set(m_instanceName.c_str()) != ESP_OK)
		ESP_LOGE(TAG, "Error while setting instance name");
}

/**
 * @brief  Add service to mDNS server
 * @param  service_type     service type (_http, _ftp, etc)
 * @param  proto            service protocol (_tcp, _udp)
 * @param  port             service port
 * @return N/A.
 */
void MDNS::addService(std::string service, std::string protocol, uint16_t port){
	mdns_service_add(m_instanceName.c_str(), service.c_str(), protocol.c_str(), port, NULL, 0);
}