/*
 * System.hpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#ifndef _SYSTEM_HPP_
#define _SYSTEM_HPP_

#include <string>
#include <time.h>
#include "esp_system.h"
#include "esp_timer.h"

class System{
public:
	System();
	~System();
	static std::string getDeviceID(void);

	/**
	 * @brief  Restart system.
	 *
	 * Function has been renamed to esp_restart.
	 * This name will be removed in a future release.
	 */
	static void restart(void){
		::esp_restart();
	}

	/**
	 * @brief Get the Random String object
	 *
	 * @return std::string 32 bytes log string
	 */
	static std::string getRandomString(void);

	/**
	 * @brief  Get the size of available heap.
	 *
	 * Note that the returned value may be larger than the maximum contiguous block
	 * which can be allocated.
	 *
	 * @return Available heap size, in bytes.
	 */
	static int getFreeHeapSize(void){
		return ::esp_get_free_heap_size();
	}

	/**
	 * @brief Get the Uptime function
	 *
	 * @return uint32_t upTime in seconds
	 */
	/* NOTE: "esp_timer_get_time() returns an int64_t so [-2³²,+2³²] is the max. It will overflow after 292000 years." */
	static uint32_t getUptime(void){ return (::esp_timer_get_time()/ 1000000); }

	/**
	 * @brief Get the UNIX Timestamp from System
	 *
	 * @return time_t
	 */
	static time_t getTime(void);

	/**
	 * @brief Set the given UNIX Timestamp to System
	 *
	 */
	static void setTime(time_t time);

	/**
	 * @brief Set the Local Time to the system
	 *
	 * @param time means a localtime in UTC timestamp format
	 * @param timezone POSIX timezones string
	 */
	static void setLocalTime(time_t time, std::string timezone);

	/**
	 * @brief Get the Timestamp in ISO8601 format with selected timezone option
	 *
	 * @param timezone POSIX timezones string
	 * @return std::string ISO8601 Timestamp
	 */
	static std::string getLocalTimestamp(const std::string timezone);

	/**
	 * @brief Get the Local Timeinfo object with selected timezone option
	 *
	 * @param [in]  timezone POSIX timezones string
	 * @param [out] timeinfo tm struct to fill
	 */
	static void getLocalTimeinfo(const std::string timezone, struct tm *timeinfo);

	/**
	 * @brief Get the Local time as UNIX Timestamp format from System
	 *
	 * @return time_t
	 */
	static time_t getLocalTime(const std::string timezone);

	/**
	 * @brief internal function to prevent memory leak
	 *
	 */
	static void initTz();

	/**
	 * @brief Set the Timezone object
	 *
	 * @param timezone POSIX timezones string
	 */
	static void setTimezone(std::string timezone);

};

#endif //_SYSTEM_HPP_