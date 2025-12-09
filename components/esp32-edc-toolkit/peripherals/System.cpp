/*
 * System.cpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#include "System.hpp"
#include "SHA256.hpp"

#include <esp_system.h>
#include <esp_log.h>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>


static bool isTzInit = false;
static char tzbuf[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; //64b buff

System::System(){

}

System::~System(){

}

std::string System::getDeviceID(void){
	uint8_t deviceID[6];

	//deviceID = new uint8_t(6);
	::esp_efuse_mac_get_default(deviceID);

	std::stringstream s;
	for (int x = 0; x < 6; x++)
    {
		s << std::uppercase << std::setfill('0') << std::setw(2) << std::hex <<  (int)deviceID[x];
	}
	return s.str();
}

std::string System::getRandomString(void){
	int length = 32;
	SHA256 sha;
	char rand[length];
	::esp_fill_random(&rand, length);
	sha.update((unsigned char*)&rand, length);
	return sha.getResultHash();
}

time_t System::getTime(void){
	setTimezone("UTC0");

	time_t now;

	time(&now);
	return now;
}

void System::setTime(time_t time){
	setTimezone("UTC0");

	struct timeval *m_tv;
	m_tv = new timeval();
	m_tv->tv_sec = time;

	settimeofday(m_tv, NULL);
	delete(m_tv);
}


void System::setLocalTime(time_t time, std::string timezone){
	struct tm timeinfo;

    getLocalTimeinfo(timezone, &timeinfo);

	char buf[8];
	strftime(buf, sizeof(buf), "%z", &timeinfo);
	std::string timeDiff(buf);

	int h = atoi(timeDiff.substr(1,2).c_str());
	int m = atoi(timeDiff.substr(3,4).c_str());

	int diff = m*60 + h*3600;

	// '-' means we have to add 'diff' to get UTC as result
	if(buf[0]=='-') {
		time += diff;
	}else{
		time -= diff;
	}

	setTime(time);
}

std::string System::getLocalTimestamp(const std::string timezone){

	struct tm timeinfo;

    getLocalTimeinfo(timezone, &timeinfo);

	std::string out = "";
    char buf[16];
  	strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
	out += buf;
	strftime(buf, sizeof(buf), "T%H:%M:%S", &timeinfo);
	out += buf;
	strftime(buf, sizeof(buf), "%z", &timeinfo);
	out += buf;
	return out;
}

void System::getLocalTimeinfo(const std::string timezone, struct tm *timeinfo){
	time_t now = getLocalTime(timezone);
    localtime_r(&now, timeinfo);
}

time_t System::getLocalTime(const std::string timezone){
	setTimezone(timezone);
	time_t now;
	time(&now);
	return now;
}

void System::initTz(){
	/* Need to clear any previous string as we don't know where it's been.
	 * This is probably a one off memory leak.
	*/
	unsetenv("TZ");

	/* Set our buffer although library may take a malloc'd copy */
	setenv( "TZ", tzbuf , 1);
	isTzInit = true;
}

void System::setTimezone(std::string timezone){
	if(!isTzInit) System::initTz();

	char *tzbuf_value;
	tzbuf_value = getenv("TZ");
	strcpy( tzbuf_value, timezone.c_str() );

    tzset();

}