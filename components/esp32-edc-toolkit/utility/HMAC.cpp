/**
 * @file HMAC.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief Easy HMAC generation. For more information, see:
 * https://tls.mbed.org/discussions/generic/how-can-i-calculate-large-file-HMAC-checksum-over-4gb
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "HMAC.hpp"
#include "mbedtls/md.h"
#include <stdio.h>
#include <string>

/**
 * @brief Construct a new HMAC::HMAC object
 *
 */
HMAC::HMAC(unsigned char* key, int keyLength, mbedtls_md_type_t type){
	mbedtls_md_type_t md_type = type;
	::mbedtls_md_init(&ctx);
	::mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
	::mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, keyLength);
}

/**
 * @brief Destroy the HMAC::HMAC object
 *
 */
HMAC::~HMAC(){
	mbedtls_md_free(&ctx);
}

/**
 * @brief Shift a new package to the HMAC calculator which is added to the calculation
 *
 * @param char next package to calculate with.
 * @param length length of that package
 */
int HMAC::update(unsigned char* payload, int payloadLength){
	return mbedtls_md_hmac_update(&ctx, (const unsigned char *) payload, payloadLength);
}

/**
 * @brief Returning the generated hash and clean the calculation session
 *
 * @return std::string result hash
 */
std::string HMAC::getResultHash(){
	unsigned char output[32];

	mbedtls_md_hmac_finish(&ctx, output);

	//convert to string
	std::string ret = "";
	char _buffer[3];  //2 Characters + \0 -> 3Bytes! TODO: write directly to std::string ret, the next two lines are crappy ;-)
	for(int i=0; i<sizeof(output); i++){
		sprintf(_buffer, "%02x", output[i]);
		ret += _buffer;
	}

	return ret;
}

/**
 * @brief Returning the generated hash and clean the calculation session
 *
 * @param output hash as char array
 */
void HMAC::getResultHash(unsigned char *output){
	mbedtls_md_hmac_finish(&ctx, output);
}