/**
 * @file SHA256.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief Easy SHA256 generation. For more information, see:
 * https://tls.mbed.org/discussions/generic/how-can-i-calculate-large-file-sha256-checksum-over-4gb
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "SHA256.hpp"
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string>

/**
 * @brief Construct a new SHA256::SHA256 object
 *
 */
SHA256::SHA256(){
	::mbedtls_sha256_init( &m_handle );
	::mbedtls_sha256_starts(&m_handle, 0);
}

/**
 * @brief Destroy the SHA256::SHA256 object
 *
 */
SHA256::~SHA256(){
	::mbedtls_sha256_free( &m_handle );
}

/**
 * @brief Shift a new package to the SHA calculator which is added to the calculation
 *
 * @param char next package to calculate with.
 * @param length length of that package
 */
void SHA256::update(unsigned char* package, int length){
	::mbedtls_sha256_update( &m_handle, package, length);
}

/**
 * @brief Returning the generated hash and clean the calculation session
 *
 * @return std::string result hash
 */
std::string SHA256::getResultHash(){
	unsigned char output[32];
	::mbedtls_sha256_finish( &m_handle, output);
	::mbedtls_sha256_free( &m_handle );

	//convert to string
	std::string ret = "";
	char _buffer[3];  //2 Characters + \0 -> 3Bytes! TODO: write directly to std::string ret, the next two lines are crappy ;-)
	for(int i=0; i<sizeof(output); i++){
		sprintf(_buffer, "%02x", output[i]);
		ret += _buffer;
	}

	return ret;
}