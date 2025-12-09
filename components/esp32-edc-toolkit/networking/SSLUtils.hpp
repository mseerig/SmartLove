/*
 * SSLUtils.h
 *
 *  Created on: Sep 16, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SSLUTILS_H_
#define COMPONENTS_CPP_UTILS_SSLUTILS_H_

#include "File.hpp"
#include <string>

class SSLUtils {
public:
	SSLUtils();
	virtual ~SSLUtils();

	void setCaCertificate(std::string certificate);
	void setCertificate(std::string certificate);
	void setKey(std::string key);

	void setCaCertificate(File file);
	void setCertificate(File file);
	void setKey(File file);

	char* getCaCertificate();
	char* getCertificate();
	char* getKey();

	bool isCaCertificate(){return m_ca_certificate_en;};
	bool isCertificate(){return m_certificate_en;};
	bool isKey(){return m_key_en;};

private:
	char* m_ca_certificate{nullptr};
	char* m_certificate{nullptr};
	char* m_key{nullptr};

	bool m_ca_certificate_en{false};
	bool m_certificate_en{false};
	bool m_key_en{false};
};

#endif /* COMPONENTS_CPP_UTILS_SSLUTILS_H_ */
