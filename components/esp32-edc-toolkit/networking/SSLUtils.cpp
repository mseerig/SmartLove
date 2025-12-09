/*
 * SSLUtils.cpp
 *
 *  Created on: Sep 16, 2017
 *      Author: kolban
 */

#include "SSLUtils.hpp"
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <fstream>

static char LOGTAG[] = "SSLUtils";

SSLUtils::SSLUtils() {
}

SSLUtils::~SSLUtils() {
}

void SSLUtils::setCaCertificate(std::string certificate) {
	if(certificate != "") m_ca_certificate_en = true;
	else m_ca_certificate_en = false;

	size_t len = certificate.length();
	m_ca_certificate = (char*)malloc(len + 1);
	memcpy(m_ca_certificate, certificate.data(), len);
	m_ca_certificate[len] = '\0';
}

void SSLUtils::setCertificate(std::string certificate) {
	if(certificate != "") m_certificate_en = true;
	else m_certificate_en = false;

	size_t len = certificate.length();
	m_certificate = (char*)malloc(len + 1);
	memcpy(m_certificate, certificate.data(), len);
	m_certificate[len] = '\0';
}

void SSLUtils::setKey(std::string key) {
	if(key != "") m_key_en = true;
	else m_key_en = false;

	size_t len = key.length();
	m_key = (char*)malloc(len + 1);
	memcpy(m_key, key.data(), len);
	m_key[len] = '\0';
}

void SSLUtils::setCaCertificate(File file) {
	setCaCertificate(file.getContent());
}

void SSLUtils::setCertificate(File file) {
	setCertificate(file.getContent());
}

void SSLUtils::setKey(File file) {
	setKey(file.getContent());
}

char* SSLUtils::getCaCertificate() {
	return m_ca_certificate;
}

char* SSLUtils::getCertificate() {
	return m_certificate;
}

char* SSLUtils::getKey() {
	return m_key;
}
