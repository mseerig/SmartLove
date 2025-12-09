/*
 * DeepSleep.hpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#ifndef _DEEPSLEEP_HPP_
#define _DEEPSLEEP_HPP_

#include "esp_sleep.h"

class DeepSleep{
public:

	static esp_sleep_wakeup_cause_t 	getWakeupCause(void);
	static esp_err_t 					setWakeupTimer(uint64_t time_in_ms);
	static esp_err_t 					disableWakeUpTimer(void);
	static void 						enterSleep(void);

	static void 						sleep(uint64_t time_in_ms);

private:

};

#endif //_DEEPSLEEP_HPP_