/**
 * @file RtcController.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief 
 * @version v0.0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "RtcController.hpp"

static char LOGTAG[]="RtcController";

RtcController::RtcController(ConfigurationManager &configurationManager, I2CArbiter &i2cArbiter, EventLog &eventLog):
  Task("RtcController", 3072, 5),
  m_configurationManager(configurationManager),
  m_i2c(i2cArbiter),
  m_rtc(BQ32000(m_i2c)),
  m_eventLog(eventLog) {

	ESP_LOGI(LOGTAG, "Starting...");

	
	esp_err_t ret = m_rtc.init(); // init and check OF Bit
	
	if(ret == ESP_OK) {

		if(m_rtc.updateSystemTimeFromRtc() != ESP_OK) {
			// communication error
			m_workingRTC = false;
			m_eventLog.push(EventLog::Event::SYSTEM_RTC_NOT_WORKING, EventLog::State::WARNING);
		}

	}else{
		// [ret=ESP_FAIL] communication error on init
 		// [ret=ERROR_TIME_CORRUPTED] time corrupted 
		m_workingRTC = false;
		m_eventLog.push(EventLog::Event::SYSTEM_RTC_NOT_WORKING, EventLog::State::WARNING);
	}

	m_updateRtcTimer = new Timer();

	Task::start();
}

RtcController::~RtcController() {
	delete m_updateRtcTimer;
	ESP_LOGI(LOGTAG, "Exiting...");
}

void RtcController::run(void *data) {

	m_updateRtcTimer->start_relative(TimerInterface::seconds(20)); //fist update after 20s since boot

	while(1) {

		if (m_updateRtcTimer->timeout()) {
			if(m_configurationManager.getTimeConfiguration().enableSNTP){
				time_t t_rtc;
				m_rtc.getTime(t_rtc);
				double diff = difftime(System::getTime(), t_rtc) ; // diff in [s]
				ESP_LOGD(LOGTAG, "Difference between System-Time an RTC is %f s.",diff);

				//allow +/- 5 seconds shift
				if(diff < -5 || diff > 5){
					m_rtc.updateRtcTimeFromSystem(); // update RTC time
				}
			}
			m_updateRtcTimer->start_relative(TimerInterface::minutes(1)); // check every minute
		}

		// HMI-Handling will be done every 10ms
		// This provides a stable timebase for input debouncing and potential LED dimming/blinking
		FreeRTOS::sleep(10);		//Sleep for 10ms
	}
}
