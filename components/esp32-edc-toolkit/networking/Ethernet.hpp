/*
 * Ethernet.h
 *
 *  Created on: 07.07.2017
 *      Author: marcel.seerig
 */

// RMII data pins are fixed:
	// TXD0 = GPIO19
	// TXD1 = GPIO22
	// TX_EN = GPIO21
	// RXD0 = GPIO25
	// RXD1 = GPIO26
	// CLK == GPIO0

#ifndef MAIN_ETHERNET_H_
#define MAIN_ETHERNET_H_

#include "NetworkEventHandler.hpp"
#include "GPIO.hpp"

#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include "esp_err.h"
#include "esp_event.h"

#include "esp_log.h"
#include "esp_eth.h"

#include "esp_netif.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "esp_eth.h"
#include <string>

class Ethernet{
public:

	//static ethernet_config_t config;

	Ethernet();
	~Ethernet();

	esp_err_t				start(void);
	esp_err_t				stop(void);

	tcpip_adapter_ip_info_t getIpInfo(void);
	std::string 			getIp(void);
	std::string 			getNetmask(void);
	std::string 			getGateway(void);
	//std::string 			getMac(void);
	std::string				getDnsInfo(esp_netif_dns_type_t type);
	void 					setDnsInfo(esp_netif_dns_type_t type, std::string ip);

	void 					setStaticIP(std::string ip, std::string gw, std::string netmask);
	void					setDynamicIP(void);

private:

	static Ethernet* 	s_Instance;
	esp_eth_handle_t 	m_hdl;

	bool		m_DHCPenabled{true};

};

//ToDo: Find a better solution for this functions.


#endif /* MAIN_ETHERNET_H_ */
