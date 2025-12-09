/**
 * @file DNS.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2021-01-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <string>
#include <esp_wifi.h>
#include <esp_log.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#include "DNS.hpp"

static const char *LOG_TAG = "DNS";

/**
 * @brief Construct a new DNS::DNS object
 *
 */
DNS::DNS(){

}

/**
 * @brief Destroy the DNS::DNS object
 *
 */
DNS::~DNS(){

}

/**
 * @brief Set a reference to a DNS server.
 *
 * Here we define a server that will act as a DNS server.  We use numdns to specify which DNS server to set
 *
 * For example:
 *
 * @code{.cpp}
 * DNS::setServer(0, "8.8.8.8");
 * DNS::setServer(1, "8.8.4.4");
 * @endcode
 *
 * @param [in] numdns The DNS number we wish to set
 * @param [in] ip The IP address of the DNS Server.
 * @return N/A.
 */
void DNS::setServer(int numdns, const std::string &ip){
	setServer(numdns, ip.c_str());
} // setServer

void DNS::setServer(int numdns, const char *ip){
	ip_addr_t dns_server;
	if (inet_pton(AF_INET, ip, &dns_server))
	{
		setServer(numdns, dns_server);
	}
} // setServer

void DNS::setServer(int numdns, ip_addr_t ip){
	ESP_LOGD(LOG_TAG, "Setting DNS[%d] to %d.%d.%d.%d", numdns, ((uint8_t *)(&ip))[0], ((uint8_t *)(&ip))[1], ((uint8_t *)(&ip))[2], ((uint8_t *)(&ip))[3]);
	::dns_setserver(numdns, &ip);
} // setServer

/**
 * @brief Dump diagnostics to the log.
 */
void DNS::dump(){
	ESP_LOGD(LOG_TAG, "DNS Dump");
	ESP_LOGD(LOG_TAG, "---------");
	char ipAddrStr[30];
	inet_ntop(AF_INET, ::dns_getserver(0), ipAddrStr, sizeof(ipAddrStr));
	ESP_LOGD(LOG_TAG, "DNS Server[0]: %s", ipAddrStr);
} // dump

/**
 * @brief Lookup an IP address by host name.
 *
 * @param [in] hostName The hostname to resolve.
 *
 * @return The IP address of the host or 0.0.0.0 if not found.
 */
struct in_addr DNS::getHostByName(const std::string &hostName){
	return getHostByName(hostName.c_str());
} // getHostByName

struct in_addr DNS::getHostByName(const char *hostName){
	struct in_addr retAddr;
	struct hostent *he = gethostbyname(hostName);
	if (he == nullptr)
	{
		retAddr.s_addr = 0;
		ESP_LOGD(LOG_TAG, "Unable to resolve %s - %d", hostName, h_errno);
	}
	else
	{
		retAddr = *(struct in_addr *)(he->h_addr_list[0]);
		ESP_LOGD(LOG_TAG, "resolved %s to %.8x", hostName, *(uint32_t *)&retAddr);
	}
	return retAddr;
} // getHostByName
