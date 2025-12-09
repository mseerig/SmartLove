/*
 * DNSlookup.hpp
 *
 *  Created on: Sep 24, 2018
 *      Author: marcel.seerig
 */

#ifndef _DNS_LOOKUP_HPP_
#define _DNS_LOOKUP_HPP_

#include <string>

/**
 * @brief DNSlookup Class.
 */
class DNSlookup{
	public:
		DNSlookup();
		~DNSlookup();
		bool getIPaddres(std::string host, int port, std::string* ip);
		bool getIPaddres(std::string host, std::string port, std::string* ip);

	private:

};

#endif