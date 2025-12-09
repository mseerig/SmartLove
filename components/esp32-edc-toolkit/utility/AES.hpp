/**
 * @file AES.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-14
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef AES_HPP
#define AES_HPP

#include <string>

#include <mbedtls/aes.h>

/**
 * @brief Simple AES calculation class.
 *
 */
class AES{
	public:

		enum class Methode{
			decryption=0,
			encryption=1
		};

		AES(AES::Methode methode, std::string key);
		~AES();

		int setKey(std::string key);

		int perform(const unsigned char input[16], unsigned char output[16]);
		int perform(std::string input, std::string& output);

	private:
		Methode 			m_methode;
		mbedtls_aes_context m_handle;
};

#endif