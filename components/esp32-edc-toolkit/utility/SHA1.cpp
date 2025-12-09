/**
 * @file SHA1.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief Easy SHA1 generation. For more information, see:
 * https://tls.mbed.org/discussions/generic/how-can-i-calculate-large-file-sha1-checksum-over-4gb
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "SHA1.hpp"
#include <mbedtls/sha1.h>
#include <stdio.h>
#include <string>

/**
 * @brief Construct a new ESP32::SHA1::SHA1 object
 *
 */
ESP32::SHA1::SHA1(){
	::mbedtls_sha1_init( &m_handle );
	::mbedtls_sha1_starts(&m_handle);
}

/**
 * @brief Destroy the ESP32::SHA1::SHA1 object
 *
 */
ESP32::SHA1::~SHA1(){
	::mbedtls_sha1_free( &m_handle );
}

/**
 * @brief Shift a new package to the SHA calculator which is added to the calculation
 *
 * @param char next package to calculate with.
 * @param length length of that package
 */
void ESP32::SHA1::update(unsigned char* package, int length){
	::mbedtls_sha1_update( &m_handle, package, length);
}

/**
 * @brief Returning the generated hash and clean the calculation session
 *
 * @return std::string result hash
 */
std::string ESP32::SHA1::getResultHash(){
	unsigned char output[32];
	::mbedtls_sha1_finish( &m_handle, output);
	::mbedtls_sha1_free( &m_handle );

	//convert to string
	std::string ret = "";
	char _buffer[3];  //2 Characters + \0 -> 3Bytes! TODO: write directly to std::string ret, the next two lines are crappy ;-)
	for(int i=0; i<sizeof(output); i++){
		sprintf(_buffer, "%02x", output[i]);
		ret += _buffer;
	}

	return ret;
}