/*
 * SNTP.cpp
 *
 *  Created on: Jan 14, 2019
 *      Author: marcel.seerig
 */

#include "SNTP.hpp"

#include <string>
#include <esp_log.h>
#include <time.h>
#include <sys/time.h>
#include <lwip/apps/sntp.h>

#include "FreeRTOS.hpp"

static const char *TAG = "SNTP";

/**
 * @brief SNTP constructor.
 */
SNTP::SNTP(){
	ESP_LOGD(TAG, "Initializing");
	setServer("pool.ntp.org");
}

void SNTP::setServer(std::string server){
	if(isEnabled()){
		ESP_LOGE(TAG, "Do not set Server while running!");
		return;
	}
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, (char*)server.c_str());
}

void SNTP::start(){
	ESP_LOGD(TAG, "Start client");
	sntp_init();
}

bool SNTP::isEnabled(){
	return (bool)sntp_enabled();
}

void SNTP::stop(){
	ESP_LOGD(TAG, "Stop client");
	sntp_stop(); // no need to check active
}

/**
 * @brief Starting the SNTP get time process.
 * This function will set the system time!
 * You can get the current with "time(&now);" afterwats.
 * @return Error case -> false.
 */
bool SNTP::timeUpdated(void){
    time_t now = 0;
    struct tm timeinfo;
	localtime_r(&now, &timeinfo);

	// wait for time to be set
	int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGD(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        FreeRTOS::sleep(250);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

	ESP_LOGD(TAG, "done");

	return (timeinfo.tm_year > (2016 - 1900));
}

/**
 * @brief SNTP destructor.
 */
SNTP::~SNTP(){
	sntp_stop();
}