/*
 * SIM900.hpp
 *
 *  Created on: 07.07.2018
 *      Author: marcel.seerig
 */

#include <string>
#include <time.h>       /* time_t, struct tm, time, mktime */
#include "esp_log.h"

#include "SIM900.hpp"
#include "FreeRTOS.hpp"
#include "UART.hpp"
#include "GPIO.hpp"

static char tag[] = "SIM900";

/**
 * @brief Construct an instance of the class.
 * @param [in] uart driver instance.
 * @return N/A.
 */
SIM900::SIM900(UART *_uart)
	:m_uart(_uart),en_pin(nullptr){
}

/**
 * @brief Construct an instance of the class.
 * @param [in] uart driver instance, enable gpio.
 * @return N/A.
 */
SIM900::SIM900(UART *_uart, gpio_num_t enPin)
	:m_uart(_uart){

	en_pin = new GPIOClass(enPin, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ONLY);
	en_pin->reset();
	FreeRTOS::sleep(1000);
}

/**
 * @brief Class instance destructor.
 * @param N/A.
 * @return N/A.
 */
SIM900::~SIM900(){
	powerOff();
	delete en_pin;
}

/**
 * @brief Power on the SIM900.
 * @param none.
 * @return none.
 */
void SIM900::powerOn(){
	if(en_pin!=nullptr){
		ESP_LOGD(tag, "Turn Power On!");
		if(getPowerState() == 0){
			en_pin->set();
			FreeRTOS::sleep(1000);
			en_pin->reset();
			while(getPowerState() == 0)	FreeRTOS::sleep(2000);
		}else ESP_LOGD(tag, "Power was still On!");
	}else ESP_LOGE(tag, "Power on is not possible, no en_pin!");
}

/**
 * @brief Power off the SIM900.
 * @param none.
 * @return none.
 */
void SIM900::powerOff(){
	if(en_pin!=nullptr){
		ESP_LOGD(tag, "Turn Power Off!");
		if(getPowerState() == 1){
			en_pin->set();
			FreeRTOS::sleep(1000);
			en_pin->reset();
		}else ESP_LOGD(tag, "Power was still Off!");
	}else ESP_LOGE(tag, "Power on is not possible, no en_pin!");
}

/**
 * @brief write the time to the SIM900
 * @param time in an standard time_t format
 * @return Error case.
 */
bool SIM900::setTime(time_t _time){
	struct tm *tm = localtime(&_time);
	char buffer [50];
	sprintf (buffer, "AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+00\"\r", tm->tm_year-100, tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	std::string cmd = buffer;
	std::string res = sendCommand(cmd);
	if(res == "OK")	return true;
	return false;
}

/**
 * @brief read the current time in time_t format
 * @param [in] current time in time_t format.
 * @return Error case.
 */
bool SIM900::getTime(time_t &_time){
	std::string res = sendCommand("AT+CCLK?\r");
	if(res=="") return false;
	std::size_t pos = res.find("OK");
	if(pos != std::string::npos){
		//delete all before "<time>
		pos = res.find("\"");
		if(pos != std::string::npos) res.erase(0,pos+1);
		else return false;

		//delete all after <time>"
		pos = res.find("\"");
		if(pos != std::string::npos) res.erase(pos,res.length());
		else return false;

		//convert string to time_t
		if(res.length() == 20){
			struct tm tm;
			strptime(res.c_str(), "%y/%m/%d,%H:%M:%S", &tm);
			_time = mktime(&tm); //encoding
			ESP_LOGD(tag, "Current time: %s", asctime (&tm));
			return true;
		}
	}
	return false;
}

/**
 * @brief Get the current power state of the SIM0900 module.
 * @param none.
 * @return in case of "on" -> true, in case of "off" -> false
 */
bool SIM900::getPowerState(){
	std::string res = sendCommand("AT\r");
	if(res != "") return true;
	return false;
}

/**
 * @brief Get the name of the Provider (stored on SIM card)
 * @param none.
 * @return string, name. in case of error return "unknown"
 */
std::string SIM900::getProviderName(){

	std::string res = sendCommand("AT+CSPN?\r");
	if(res=="") return "unknown";

	if(res.substr(res.find("OK")) == "OK"){
		try{
			res.erase(0, res.find(": ")+3);
			res.erase(res.find("\""), res.length());
		}catch (...) {
			res="unknown";
		}
	}else{
		res="unknown";
	}

	return res;
}

/**
 * @brief Returns the signal strength.
 * @param none.
 * @return Possible returns: -1=Error/No Signal, 1=Marginal, 2=OK, 3=Good, 4=Excellent.
 */
int SIM900::getSignalStrength(){

	std::string res = sendCommand("AT+CSQ\r");
	if(res != ""){
		if(res.substr(res.find("OK")) == "OK"){

			float signal;
			try{
				res.erase(res.find("OK")-4, res.length());
				res.erase(0, res.find(": ")+2);

				signal = strtof((res).c_str(),0);
			}catch (...){
				ESP_LOGE(tag, "Undefined Request!");
				return -1;
			}

			// Definitions by SIM900 manual
			if(signal>2 && signal<10) return 1; //Marginal
			if(signal>=10 && signal<15) return 2; //OK
			if(signal>=15 && signal<20) return 3; //Good
			if(signal>=20) return 4; //Excellent
		}
	}

	ESP_LOGD(tag, "getSignalStrength -> ERROR");
	return -1; // No Signal
}

bool SIM900::isRegiteredToNetwork(){

	std::string res = sendCommand("AT+CGREG?\r");
	if(res != ""){
		if(res.substr(res.find("OK")) == "OK"){
			res.erase(0, res.find(": ")+2);
			if (res.length()>3) res.erase(3, res.length());
			else return false;

			if(res == "0,0") return false;
			if(res == "0,2") return false;
			if(res == "0,3") return false;
			if(res == "0,4") return false;
		}
	}

	ESP_LOGD(tag, "isRegiteredToNetwork -> '%s'", res.c_str());

	return true;
}

/**
 * @brief Send a SMS to the given phone number.
 * @param Client phone number and the SMS text.
 * @return Error case.
 */
bool SIM900::sendSMS(std::string phoneNumber, std::string msg){

	ESP_LOGD(tag, "Send SMS to '%s':\"%s\"", phoneNumber.c_str(), msg.c_str());

	std::string res = sendCommand("AT+CMGF=1\r"); //Delete SMS Message
	if(res != "OK") return false;

	res = sendCommand("AT+CMGS=\""+phoneNumber+"\"\r\n"); //Send SMS Message
	//if(res != "> ") return false;

	FreeRTOS::sleep(1000);

	res = sendCommand(msg, true);
	//if(res != msg+'\r') return false;

	FreeRTOS::sleep(500);

	std::string CR = "";
	CR += (char)26;
	res = sendCommand(CR, false, 10000);
	if((res == "") || (res.find("OK") == std::string::npos)) return false;

	return true;
}

/**
 * @brief Send an AT Command to the SIM900 Module.
 * @param At Command, bit to return the unparsed raw answer.
 * @return Returns the response without the original command and without the last "\r\r\n".
 *  For Example:
 *  - Send "AT\r" -> Answer "AT\r\r\r\nOK\r\r\n"
 *    ... Return "OK"
 *  - Send "AT+CSQ\r" -> Answer "AT+CSQ\r\r\r\n+CSQ: 22,0\r\r\n\r\r\nOK\r\r\n"
 *    ... Return "+CSQ: 22,0\r\r\n\r\r\nOK"
 */
std::string SIM900::sendCommand(std::string cmd, bool getRawAnswer, int addDelay){
	m_uart->flush(); //clear rx buffer

	ESP_LOGD(tag, "sendCommand: \"%s\"", cmd.c_str());
	m_uart->send(cmd.c_str());

	FreeRTOS::sleep(WAIT_FOR_ANSWER_DELAY+addDelay);

	std::string res = "";
	while(m_uart->hasData()){
		res += (char)m_uart->readByte();
		FreeRTOS::sleep(15);
	}

	//FreeRTOS::sleep(WAIT_FOR_ANSWER_DELAY); //delay to the next cmd

	ESP_LOGD(tag, "RAW Answer: \"%s\"", res.c_str());
	if(getRawAnswer == true) return res;

	//delete first "AT[...]\r\r\n"<ANSWER>
	std::string::size_type found = res.find("\r\r\n");
  	if (found!=std::string::npos) res.erase(0,found+3);
	else if(res.length()>3) res.erase(res.begin(),res.begin()+2);

	//delete last <ANSWER>"\r\r\n"
	if(res.length()>3) res.erase(res.end()-2,res.end());

	ESP_LOGD(tag, "Answer: \"%s\"", res.c_str());
	return res;
}