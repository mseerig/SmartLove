/**
 * @file HMAC.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef HMAC_HPP
#define HMAC_HPP

#include <string>

#include "mbedtls/md.h"

/**
 * @brief Simple HMAC calculation class.
 *
 */
class HMAC{
	public:
		HMAC(unsigned char* key, int keyLength, mbedtls_md_type_t type=MBEDTLS_MD_SHA256);
		~HMAC();

		int 		update(unsigned char* payload, int payloadLength);
		std::string getResultHash();
		void 		getResultHash(unsigned char *output);

	private:
		mbedtls_md_context_t ctx;
};

#endif