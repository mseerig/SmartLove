/**
 * @file RtcController.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief 
 * @version v0.0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef RTC_CONTROLLER_H_
#define RTC_CONTROLLER_H_


#include "FreeRTOS.hpp"
#include "Task.hpp"
#include "esp_log.h"

#include "I2CArbiter.hpp"
#include "BQ32000.hpp"
#include "Definitions.hpp"
#include "Timer.hpp"
#include "TimerInterface.h"
#include "System.hpp"
#include "EventLog.hpp"

#include "ConfigurationManager.hpp"

class RtcController:public Task {
public:
	RtcController(ConfigurationManager &configurationManager, I2CArbiter &i2cArbiter, EventLog &eventLog);
	~RtcController(void);

	BQ32000* getRtc(void){return &m_rtc;}
	bool isRtcWorking(void){return m_workingRTC;}

private:
	void run(void *data);

	ConfigurationManager	&m_configurationManager;
	I2CArbiter				&m_i2c;
	BQ32000					m_rtc;
	EventLog 				&m_eventLog;

	TimerInterface * 		m_updateRtcTimer{nullptr};
	bool					m_workingRTC{true};

};

#endif
