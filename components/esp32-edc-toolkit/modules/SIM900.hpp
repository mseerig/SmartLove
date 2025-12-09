/*
 * SIM900.hpp
 *
 *  Created on: 07.07.2018
 *      Author: marcel.seerig
 */

#ifndef SIM900_HPP_
#define SIM900_HPP_

#include <string>
#include <time.h>       /* time_t, struct tm, time, mktime */
#include "UART.hpp"
#include "GPIO.hpp"

#define WAIT_FOR_ANSWER_DELAY	1000    //in ms
//#define SIM900_TIMEOUT			10000  //in ms


class SIM900{
	public:
		SIM900(UART *_uart);
		SIM900(UART *_uart, gpio_num_t enPin);
		~SIM900();

		void powerOn();
		void powerOff();
		bool getTime(time_t &_time);
		bool setTime(time_t _time);
		bool getPowerState();
		int  getSignalStrength();
		bool isRegiteredToNetwork();
		std::string getProviderName();
		bool sendSMS(std::string phoneNumber, std::string msg);

	private:
		UART* m_uart;
		GPIOClass* en_pin;

		std::string sendCommand(std::string cmd, bool getRawAnswer=false, int addDelay = 1);

};

#endif