/*
 * DNSlookup.cpp
 *
 *  Created on: Sep 24, 2018
 *      Author: marcel.seerig
 */

#include <string>
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "DNSlookup.hpp"

static const char *TAG = "DNSlookup";

/**
 * @brief DNSlookup constructor.
 */
DNSlookup::DNSlookup(){

}

/**
 * @brief DNSlookup destructor.
 */
DNSlookup::~DNSlookup(){

}

/**
 * @brief Decode a hostname by a given port to an ip-addres.
 * Possible host name format: "www.examlpe.de", "example.de".
 * Not allowed: "123.456.789.123" (ip), "http://<host>"!
 * @param [in] in The host as string.
 * @param [in] in The port as int.
 * @param [out] out The resulting ip-addres as string.
 * @return Error case -> false.
 */
bool DNSlookup::getIPaddres(std::string host, int port, std::string* ip){
	char buffer [5];
	sprintf (buffer, "%d", port);
	return getIPaddres(host, buffer, ip);
}

/**
 * @brief Decode a hostname by a given port to an ip-addres.
 * Possible host name format: "www.examlpe.de", "example.de".
 * Not allowed: "123.456.789.123" (ip), "http://<host>"!
 * @param [in] in The host as string.
 * @param [in] in The port as string.
 * @param [out] out The resulting ip-addres as string.
 * @return Error case -> false.
 */
bool DNSlookup::getIPaddres(std::string host, std::string port, std::string* ip){
	struct addrinfo hints;// = {
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *res;
	struct in_addr *addr;

	int err = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);

	if (err != 0 || res == NULL){
		ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
		return false;
	}

	// Code to print the resolved IP.
	// Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code
	addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
	(*ip) = inet_ntoa(*addr);
	ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", ip->c_str());

	return true;
}