/**
 * @file DNS.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2021-01-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TOOLKIT_DNS_H_
#define TOOLKIT_DNS_H_

#include <string>
#include <esp_wifi.h>
#include <lwip/dns.h>

class DNS{

	public:
		DNS();
		~DNS();
		static void setServer(int numdns, const std::string &ip);
		static void setServer(int numdns, const char *ip);
		static void setServer(int numdns, ip_addr_t ip);
		static struct in_addr getHostByName(const std::string &hostName);
		static struct in_addr getHostByName(const char *hostName);
		static void dump();
};

#endif