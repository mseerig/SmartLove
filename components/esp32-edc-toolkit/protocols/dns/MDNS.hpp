/*
 * MDNS.hpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#ifndef _MDNS_HPP_
#define _MDNS_HPP_

#include <string>

class MDNS{
	public:
		MDNS(std::string hostname, std::string instanceName="");
		~MDNS();
		void refresh(void);
		void setHostname(std::string hostname);
		void setInstanceName(std::string instanceName);
		void addService(std::string service, std::string protocol, uint16_t port);

	private:
		std::string m_hostname;
		std::string m_instanceName;
};

#endif //_MDNS_HPP_