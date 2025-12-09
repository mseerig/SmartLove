/*
 * SNTP.hpp
 *
 *  Created on: Jan 14, 2019
 *      Author: marcel.seerig
 */

#ifndef _SNTP_HPP_
#define _SNTP_HPP_

#include <string>

/**
 * @brief SNTP Class.
 */
class SNTP{
	public:
		SNTP();
		~SNTP();
		static void setServer(std::string server);
		static void start();
		static void stop();
		static bool isEnabled();
		static bool timeUpdated(void);

	private:

};

#endif