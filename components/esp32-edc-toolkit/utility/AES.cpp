/**
 * @file AES.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-14
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "AES.hpp"
#include <mbedtls/aes.h>
#include <stdio.h>
#include <string>
#include <esp_log.h>

/**
 * @brief Construct a new AES::AES object
 *
 */
AES::AES(AES::Methode methode, std::string key){
	m_methode = methode;
	::mbedtls_aes_init( &m_handle );
	unsigned int length = key.length()*8;

	if(m_methode == AES::Methode::encryption){
		::mbedtls_aes_setkey_enc( &m_handle, (const unsigned char*) key.c_str(), length);
	}else{
		::mbedtls_aes_setkey_dec( &m_handle, (const unsigned char*) key.c_str(), length);
	}
}

/**
 * @brief Destroy the AES::AES object
 *
 */
AES::~AES(){
	::mbedtls_aes_free( &m_handle );
}

int AES::perform(const unsigned char input[16], unsigned char output[16]){
	return ::mbedtls_aes_crypt_ecb( &m_handle, (int)m_methode, input, output);
}

int AES::perform(std::string input, std::string& output){
	output = "";
	if(input.length()%16 != 0) return -1;

	for(int i = 0; i< input.length()/16; i++){
		const char* in = input.substr( i*16, (i+1)*16).c_str();

		unsigned char out[16];
		int ret = perform((const unsigned char*)in, out);
		if(ret != 0) return ret;
		output += (char*)out;
	}
	return 0;
}
