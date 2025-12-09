/*
 * HttpClient.hpp
 *
 *  Created on: 14.02.2019
 *      Author: marcel.seerig
 */

#ifndef _HTTP_CLIENT_HPP_
#define _HTTP_CLIENT_HPP_

#include <string>
#include <vector>

#include "esp_http_client.h"

class HttpClientEventHandler{
	public:
		HttpClientEventHandler(void* data=nullptr);

		virtual void onError(esp_http_client_event_t *event);
		virtual void onConnected(esp_http_client_event_t *event);
		virtual void onHeaderSent(esp_http_client_event_t *event);
		virtual void onHeader(esp_http_client_event_t *event);
		virtual void onData(esp_http_client_event_t *event);
		virtual void onFinish(esp_http_client_event_t *event);
		virtual void onDisconnected(esp_http_client_event_t *event);

	private:
		void* m_data;
};

class HttpClient{
public:
	HttpClient();
	HttpClient(esp_http_client_config_t config);
	~HttpClient();

	//befor perform
	void setEventHandler(HttpClientEventHandler* eventHandler);
	void setBuffersize(int buffersize);
	void setTimeout(int timeout);
	void setUrl(std::string url);
	void setPostData(std::string postData);
	void setHeader(std::string key, std::string value);
	esp_err_t deleteHeader(std::string key);

	//perform
	esp_err_t perform(esp_http_client_method_t method);

	//after perform
	int getStatusCode(void);
	int getContentLength(void);
	int getBodyPart(char* buffer, int length);

private:
	/* data */
	bool m_init{false};
	std::string m_postData{""};
	std::string m_url{""};
	int m_buffersize{1024};
	int m_timeout{10*1000};
	std::vector<std::pair<std::string,std::string>> m_httpHeadder;
	//esp_http_client_config_t m_config;
	esp_http_client_handle_t m_client;
	HttpClientEventHandler* m_handler;

	static esp_err_t event_handler(esp_http_client_event_t *evt);
	void setDefaults(esp_http_client_config_t *config);
};

#endif