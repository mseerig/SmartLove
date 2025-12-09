/*
 * drivers_BQ30000.h
 *
 *  Created on: 16.06.2017
 *      Author: marcel.seerig
 */

#ifndef MAIN_DRIVERS_BQ32000_H_
#define MAIN_DRIVERS_BQ32000_H_

#include <stdio.h>      /* printf, scanf */
#include <time.h>       /* time_t, struct tm, time, mktime */
#include <string>
#include <sys/time.h>

#include <esp_log.h>
#include <esp_err.h>
#include "I2CArbiter.hpp"

//Address has only 7bit, because the "least significant bit" is reserved for ACK/NAK.
#define BQ32000_ADDR  		    0b1101000     /*!< slave address for BQ32000 sensor */
#define BQ32000_WRITE_BIT  		0x00    /*!< I2C master write */
#define BQ32000_READ_BIT   		0x01 	/*!< I2C master read */
#define BQ32000_ACK_CHECK_EN   	0x01    /*!< I2C master will check ack from slave*/
#define BQ32000_ACK_CHECK_DIS  	0x00    /*!< I2C master will not check ack from slave */
#define BQ32000_ACK_VAL    		0x00    /*!< I2C ack value */
#define BQ32000_NACK_VAL  		0x01    /*!< I2C nack value */

#define BQ32000_SECONDS 	    0x00	/*!< Clock seconds and STOP bit */
#define BQ32000_MINUTES 	    0x01	/*!< Clock minutes */
#define BQ32000_CENT_HOURS 	    0x02	/*!< Clock hours, century, and CENT_EN bit */
#define BQ32000_DAY		 	    0x03	/*!< Clock day */
#define BQ32000_DATE	 	    0x04	/*!< Clock date */
#define BQ32000_MONTH	 	    0x05	/*!< Clock month */
#define BQ32000_YEARS	 	    0x06	/*!< Clock years */
#define BQ32000_CAL_CFG1 	    0x07	/*!< Calibration and configuration */
#define BQ32000_TCH2	 	    0x08	/*!< Trickle charge enabl */
#define BQ32000_CFG2	 	    0x09	/*!< Configuration 2 */

#define BQ32000_SF_KEY_1 	    0x20	/*!< Special function key 1 */
#define BQ32000_SF_KEY_2	    0x21	/*!< Special function key 2 */
#define BQ32000_SFR 			0x22	/*!< Special function register */

#define ERROR_TIME_CORRUPTED 	-99

class BQ32000 {
public:
	/**
	 * @brief Construct an instance of the class.
	 *  @param [in] i2c driver instance
	 * @return N/A.
	 */
	BQ32000(I2CArbiter& i2c);

	/**
	 * @brief Class instance destructor.
	 * @param N/A.
	 * @return N/A.
	 */
	~BQ32000();

	/**
	 * @brief Initial command for get the essential runtime information from RTC.
	 * @param N/A.
	 * @return
	 * 	-	ESP_OK -> RTC has no failure since last usage detected
	 * 	-	ESP_FAIL -> RTC has a failure. (Battery issue?)
	 */
	int init(void);

	int updateSystemTimeFromRtc();
	int updateRtcTimeFromSystem();

	/**
	 * @brief read the current time in time_t format
	 * @param N/A.
	 * @return Error case.
	 */
	int getTime(time_t &_time);

	/**
	 * @brief write the time to the bq32000
	 * @param time in an standard time_t format
	 * @return Error case.
	 */
	int setTime(time_t time);

	/**
	 * @brief read all time and flags registers
	 * @param N/A.
	 * @return Error case.
	 */
	int readTime(void);

	/**
	 * @brief set all time registers and flags to an new time without error
	 * @param N/A.
	 * @return Error case.
	 */
	int writeTime(void);

	/**
	 * @brief read all Flgs from the bq32000 and store it to the class values
	 * @param N/A.
	 * @return Error case.
	 */
	int readFlags(void);

	/**
	 * @brief get Oscillator Fail Flag (Oscillator stopped or not?)
	 * @param N/A.
	 * @return boolean [set/not set]
	 */
	bool getOscillatorFailFlag(void) {
		return m_of;
	}

	/**
	 * @brief get Oscillator Stop Flag (RTC error detection)
	 * @param N/A.
	 * @return boolean [set/not set]
	 */
	bool getOscillatorStopFlag(void) {
		return m_stop;
	}

	/**
	 * @brief get CenturyEn Flag (who knows what that dose?)
	 * @param N/A.
	 * @return boolean [set/not set]
	 */
	bool getCenturyEnFlag(void) {
		return m_cent_en;
	}

	/**
	 * @brief stop the Oscillator with the stop flag
	 * @param N/A.
	 * @return Error case.
	 */
	int setOscillatorStopFlag(void);

	/**
	 * @brief set the CenturyEn flag (who knows what that dose?)
	 * @param N/A.
	 * @return Error case.
	 */
	int setCenturyEnFlag(void);

	/**
	 * @brief reset failure detection flag
	 * @param N/A.
	 * @return Error case.
	 */
	int resetOscillatorFailFlag(void);

	/**
	 * @brief reset the CenturyEn flag (who knows what that dose?)
	 * @param N/A.
	 * @return Error case.
	 */
	int resetCenturyEnFlag(void);

	/**
	 * @brief set the oscillator to active (time is running)
	 * @param N/A.
	 * @return Error case.
	 */
	int resetOscillatorStopFlag(void);

private:
	//buffer for flags
	bool m_stop; //Stop Bit
	bool m_of; //OscillatorFailFlag
	bool m_cent_en; // CenturyEnFlag

	I2CArbiter& i2c; //extern i2c instance

	//buffer for time
	time_t m_rawtime;
	struct tm * m_timeinfo;

	/**
	 * @brief write a single register from the bq32000
	 * @param [in] register address
	 * @param [out] data to write in the register
	 * @return Error case.
	 */
	int write_reg(uint8_t reg, uint8_t dat);

	/**
	 * @brief read a single register from the bq32000
	 * @param [in] register address
	 * @param [in] data variable for response
	 * @return Error case.
	 */
	int read_reg(uint8_t reg, uint8_t *dat);

};

#endif /* MAIN_DRIVERS_BQ32000_H_ */
