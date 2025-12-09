/**
 * @file SHA1.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-03-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef SHA1_HPP
#define SHA1_HPP

#include <string>

#include <mbedtls/sha1.h>
namespace ESP32
{
	/**
	 * @brief Simple SHA1 calculation class.
	 *
	 */
	class SHA1{
		public:
			SHA1();
			~SHA1();

			void update(unsigned char* package, int length);
			std::string getResultHash();

		private:
			mbedtls_sha1_context m_handle;
	};

} // namespace ESP32


#endif