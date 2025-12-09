/**
 * @file SHA256.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef SHA256_HPP
#define SHA256_HPP

#include <string>

#include <mbedtls/sha256.h>

/**
 * @brief Simple SHA256 calculation class.
 *
 */
class SHA256{
	public:
		SHA256();
		~SHA256();

		void update(unsigned char* package, int length);
		std::string getResultHash();

	private:
		mbedtls_sha256_context m_handle;
};

#endif