/*
 * BQ3200.cpp
 *
 *  Created on: 13.06.2017
 *      Author: marcel.seerig
 */
#include "BQ32000.hpp"
#include "System.hpp"

static const char* TAG = "BQ32000";

#include "sdkconfig.h"

/**
 * @brief Construct an instance of the class.
 * @param [in] i2c driver instance
 * @return N/A.
 */
BQ32000::BQ32000(I2CArbiter& i2c) :
		i2c(i2c) {
	m_stop = 0;
	m_of = 0;
	m_cent_en = 0;
	m_rawtime = 0;
	m_timeinfo = localtime(&m_rawtime);
}

/**
 * @brief Class instance destructor.
 * @param N/A.
 * @return N/A.
 */
BQ32000::~BQ32000() {
	// I2C Stop !!!
}

/**
 * @brief Initial command for get the essential runtime information from RTC.
 * @param N/A.
 * @return
 * 	-	ESP_OK -> RTC has no failure since last usage detected
 * 	-	ESP_FAIL -> RTC has a failure. (Battery issue?)
 */
int BQ32000::init(void) {
	int ret = ESP_FAIL;

	ret = BQ32000::readFlags();
	if (ret != ESP_OK) return ret; // return i2c error

	if (getOscillatorFailFlag()) {
		ESP_LOGE(TAG, "RTC Time is corrupted!!!\n");
		ret = BQ32000::resetOscillatorFailFlag();
		if (ret != ESP_OK) return ret; // return i2c error

		return ERROR_TIME_CORRUPTED; // return rtc error
	}

	return ret; //return ESP_OK
}

/**
 * @brief read a single register from the bq32000
 * @param [in]  register address
 * @param [out] data variable for response
 * @return Error case.
 */
int BQ32000::read_reg(uint8_t reg, uint8_t *dat) {
	int ret = ESP_FAIL;

	ret = i2c.acquireBus();
	if (ret == ESP_OK) i2c.setAddress(BQ32000_ADDR);
	if (ret == ESP_OK) ret = i2c.beginTransaction();
	if (ret == ESP_OK) ret = i2c.write(reg, I2C_MASTER_ACK);
	if (ret == ESP_OK) ret = i2c.endTransaction();
	if (ret == ESP_OK) ret = i2c.beginTransaction();
	if (ret == ESP_OK) ret = i2c.read(dat, I2C_MASTER_NACK);

	// endTransaction anyway!
 	if(i2c.endTransaction() == ESP_OK && ret == ESP_OK){
		i2c.releaseBus();
		return ESP_OK;
	}
	i2c.releaseBus();	//release bus anyway
	return ESP_FAIL;
}

/**
 * @brief write a single register from the bq32000
 * @param [in] register address
 * @param [in] data to write in the register
 * @return Error case.
 */
int BQ32000::write_reg(uint8_t reg, uint8_t dat) {
	int ret = ESP_FAIL;

	uint8_t buff[2] = { reg, dat };
	ret = i2c.acquireBus();

	if (ret == ESP_OK) i2c.setAddress(BQ32000_ADDR);
	if (ret == ESP_OK) ret = i2c.beginTransaction();
	if (ret == ESP_OK) ret = i2c.write(buff, 2, I2C_MASTER_ACK);

	// endTransaction anyway!
	if(i2c.endTransaction() == ESP_OK && ret == ESP_OK){
		i2c.releaseBus();
		return ESP_OK;
	}
	i2c.releaseBus();	//release bus anyway
	return ESP_FAIL;
}

/**
 * @brief read all Flgs from the bq32000 and store it to the class values
 * @param N/A.
 * @return Error case.
 */
int BQ32000::readFlags(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_SECONDS, &_data);
	if (ret != ESP_OK) return ret;
	m_stop = ((_data & 0b10000000) >> 7);

	ret = BQ32000::read_reg(BQ32000_MINUTES, &_data);
	if (ret != ESP_OK) return ret;
	m_of = ((_data & 0b10000000) >> 7);

	ret = BQ32000::read_reg(BQ32000_CENT_HOURS, &_data);
	if (ret != ESP_OK) return ret;
	m_cent_en = ((_data & 0b10000000) >> 7);

	ESP_LOGD(TAG, "Flags: stop: %d, of: %d, cent: %d", m_stop, m_of, m_cent_en);

	return ret;
}

/**
 * @brief stop the Oscillator with the stop flag
 * @param N/A.
 * @return Error case.
 */
int BQ32000::setOscillatorStopFlag(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_SECONDS, &_data);
	if (ret == ESP_OK) {
		_data = _data | 0b10000000;
		ret = BQ32000::write_reg(BQ32000_SECONDS, _data);
	}
	return ret;
}

/**
 * @brief set the CenturyEn flag (who knows what that dose?)
 * @param N/A.
 * @return Error case.
 */
int BQ32000::setCenturyEnFlag(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_CENT_HOURS, &_data);
	if (ret == ESP_OK) {
		_data = _data | 0b10000000;
		ret = BQ32000::write_reg(BQ32000_CENT_HOURS, _data);
	}
	return ret;
}

/**
 * @brief reset failure detection flag
 * @param N/A.
 * @return Error case.
 */
int BQ32000::resetOscillatorFailFlag(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_MINUTES, &_data);
	if (ret == ESP_OK) {
		_data = _data & 0b01111111;
		ret = BQ32000::write_reg(BQ32000_MINUTES, _data);
	}
	return ret;
}

/**
 * @brief set the oscillator to active (time is running)
 * @param N/A.
 * @return Error case.
 */
int BQ32000::resetOscillatorStopFlag(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_SECONDS, &_data);
	if (ret == ESP_OK) {
		_data = _data & 0b01111111;
		ret = BQ32000::write_reg(BQ32000_SECONDS, _data);
	}
	return ret;
}

/**
 * @brief reset the CenturyEn flag (who knows what that dose?)
 * @param N/A.
 * @return Error case.
 */
int BQ32000::resetCenturyEnFlag(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	ret = BQ32000::read_reg(BQ32000_CENT_HOURS, &_data);
	if (ret == ESP_OK) {
		_data = _data & 0b01111111;
		ret = BQ32000::write_reg(BQ32000_CENT_HOURS, _data);
	}
	return ret;
}

/**
 * @brief set all time registers and flags to an new time without error
 * @param N/A.
 * @return Error case.
 */
int BQ32000::writeTime(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	uint8_t zehner, einer;

	/* WRITE SECONDS */
	zehner = ((m_timeinfo->tm_sec / 10) << 4);
	einer = m_timeinfo->tm_sec % 10;
	_data = (0 << 7) | zehner | einer;
	ret = BQ32000::write_reg(BQ32000_SECONDS, _data);
	if (ret != ESP_OK) return ret;

	/* WRITE MINUTES */
	zehner = ((m_timeinfo->tm_min / 10) << 4);
	einer = m_timeinfo->tm_min % 10;
	_data = (0 << 7) | zehner | einer;
	ret = BQ32000::write_reg(BQ32000_MINUTES, _data);
	if (ret != ESP_OK) return ret;

	/* WRITE HOURS */
	zehner = ((m_timeinfo->tm_hour / 10) << 4);
	einer = m_timeinfo->tm_hour % 10;
	_data = (0 << 7) | ((/*cent*/0) << 6) | zehner | einer;
	ret = BQ32000::write_reg(BQ32000_CENT_HOURS, _data);
	if (ret != ESP_OK) return ret;

	/* WRITE DAY */
	//_data = time.day;
	//BQ32000::write_reg(BQ32000_DAY, _data);
	/* WRITE DATE */
	zehner = ((m_timeinfo->tm_mday / 10) << 4);
	einer = m_timeinfo->tm_mday % 10;
	_data = zehner | einer;
	ret = BQ32000::write_reg(BQ32000_DATE, _data);
	if (ret != ESP_OK) return ret;

	/* WRITE MONTH */
	zehner = (((m_timeinfo->tm_mon + 1) / 10) << 4);
	einer = (m_timeinfo->tm_mon + 1) % 10;
	_data = zehner | einer;
	ret = BQ32000::write_reg(BQ32000_MONTH, _data);
	if (ret != ESP_OK) return ret;

	/* WRITE YEAR */
	zehner = (((m_timeinfo->tm_year) / 10) << 4);
	einer = (m_timeinfo->tm_year) % 10;
	_data = zehner | einer;
	ret = BQ32000::write_reg(BQ32000_YEARS, _data);
	
	return ret;
}

/**
 * @brief read all time and flags registers
 * @param N/A.
 * @return Error case.
 */
int BQ32000::readTime(void) {
	int ret = ESP_FAIL;

	uint8_t _data;
	uint8_t zehner, einer;

	/* READ SECONDS */
	ret = BQ32000::read_reg(BQ32000_SECONDS, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b01110000) >> 4);  // 10_SECONS
	einer = (_data & 0b00001111); // 1_SECOND
	m_timeinfo->tm_sec = (zehner * 10 + einer);
	m_stop = ((_data & 0b10000000) >> 7);

	/* READ MINUTES */
	ret = BQ32000::read_reg(BQ32000_MINUTES, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b01110000) >> 4);  // 10_MINUTE
	einer = (_data & 0b00001111); // 1_MINUTE
	m_timeinfo->tm_min = (zehner * 10 + einer);
	m_of = ((_data & 0b10000000) >> 7);

	/* READ HOURES */
	ret = BQ32000::read_reg(BQ32000_CENT_HOURS, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b00110000) >> 4);  // 10_HOUR
	einer = (_data & 0b00001111); // 1_HOUR
	m_timeinfo->tm_hour = (zehner * 10 + einer);
	m_cent_en = ((_data & 0b10000000) >> 7);
	//cent = ((data & 0b01000000) >> 6);

	/* READ DATE */
	ret = BQ32000::read_reg(BQ32000_DATE, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b00110000) >> 4);  // 10_DATE
	einer = (_data & 0b00001111); // 1_DATE
	m_timeinfo->tm_mday = (zehner * 10 + einer);

	/* READ MONTH */
	ret = BQ32000::read_reg(BQ32000_MONTH, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b00010000) >> 4);  // 10_DATE
	einer = (_data & 0b00001111); // 1_DATE
	m_timeinfo->tm_mon = (zehner * 10 + einer) - 1;

	/* READ YEAR */
	ret = BQ32000::read_reg(BQ32000_YEARS, &_data);
	if (ret != ESP_OK) return ret;
	zehner = ((_data & 0b11110000) >> 4);  // 10_DATE
	einer = (_data & 0b00001111); // 1_DATE
	m_timeinfo->tm_year = (zehner * 10 + einer);
	

	return ret;
}

/**
 * @brief read the current time in time_t format
 * @param [in] current time in time_t format.
 * @return Error case.
 */
int BQ32000::getTime(time_t &_time) {
	int ret = BQ32000::readTime();
	_time = mktime(m_timeinfo); //encoding
	return ret;
}

/**
 * @brief write the time to the bq32000
 * @param time in an standard time_t format
 * @return Error case.
 */
int BQ32000::setTime(time_t _time) {

	m_timeinfo = localtime(&_time);
	return BQ32000::writeTime();
}

/**
 * @brief read the current time in time_t format
 * @param [in] current time in time_t format.
 * @return Error case.
 */
int BQ32000::updateSystemTimeFromRtc() {

	if(BQ32000::readTime() == ESP_OK){
		m_rawtime = mktime(m_timeinfo); //encoding
		System::setTime(m_rawtime);

		ESP_LOGD(TAG, "set system time to %d : %s", (int)m_rawtime, asctime(m_timeinfo));
		return ESP_OK;
	}
	ESP_LOGE(TAG, "Can't read Time from RTC!");

	return ESP_FAIL;
}

/**
 * @brief write the time to the bq32000
 * @param time in an standard time_t format
 * @return Error case.
 */
int BQ32000::updateRtcTimeFromSystem() {
	time_t time = System::getTime();
	m_timeinfo = localtime(&time);
	ESP_LOGD(TAG, "set RTC time to %s", asctime(m_timeinfo));
	return BQ32000::writeTime();
}