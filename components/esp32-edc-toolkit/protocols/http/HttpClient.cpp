/*
 * HttpClient.cpp
 *
 *  Created on: 14.02.2019
 *      Author: marcel.seerig
 */

#include "HttpClient.hpp"

#include "GeneralUtils.hpp"
#include "esp_log.h"
#include "esp_http_client.h"
#include <string>

static const char *TAG = "HttpClient";
static const char *EVENT = "HttpClientEventHandler";

HttpClient::HttpClient(){

	m_handler = new HttpClientEventHandler; //config event_handle

	ESP_LOGD(TAG, "HttpClient prepared...");
}

HttpClient::HttpClient(esp_http_client_config_t config){

	m_handler = new HttpClientEventHandler; //config event_handle
	ESP_LOGD(TAG, "HttpClient prepared...");
}

/**
 * @brief set the eventhandler for this HttpClient instance.
 * @param eventHandler pointer to the handler instance.
 */
void HttpClient::setEventHandler(HttpClientEventHandler* eventHandler){
	m_handler = eventHandler;
}

void HttpClient::setBuffersize(int buffersize){
	m_buffersize = buffersize;
}

void HttpClient::setTimeout(int timeout){
	m_timeout = timeout;
}

void HttpClient::setPostData(std::string postData){
	m_postData = postData;
}

void HttpClient::setUrl(std::string url){
	m_url = url;
}

/**
 * @brief      Set http request header, this function must be called after construct and before any
 *             perform function
 * @param[in]  key     The header key
 * @param[in]  value   The header value
 */
void HttpClient::setHeader(std::string key, std::string value){
	m_httpHeadder.push_back(make_pair(key, value));
}

/**
 * @brief      Delete http request header
 * @param[in]  key     The key
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t HttpClient::deleteHeader(std::string key){
	for(int i = 0; i<m_httpHeadder.size(); i++){
		if(key == std::get<0>(m_httpHeadder.at(i))){
			m_httpHeadder.erase(m_httpHeadder.begin()+i);
			return ESP_OK;
		}
	}
	return ESP_FAIL;
}

/**
 * @brief      perform http costum request
 * @param[in]  method    A HTTP metode from esp_http_client_method_t
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t HttpClient::perform(esp_http_client_method_t method){
	/* NOTE:
	* - We have to create the new session here, because the eventhandler may changed.
	* - We have to reinit the session, because an idf-bug
	*/
	//create new session
	esp_http_client_config_t config;
	config.url = m_url.c_str();
	config.host = nullptr;
	config.port = 80;
	config.username = nullptr;
	config.password = nullptr;
	config.auth_type = HTTP_AUTH_TYPE_NONE;
	config.path = nullptr;
	config.query = nullptr;
	config.cert_pem = nullptr;
	config.client_cert_pem = nullptr;
	config.client_key_pem = nullptr;
	config.method = method;
	config.timeout_ms = m_timeout;
	config.disable_auto_redirect = false;
	config.max_redirection_count = 0;
	config.event_handler = event_handler;
	config.transport_type = HTTP_TRANSPORT_UNKNOWN;
	config.buffer_size = m_buffersize;
	config.user_data = m_handler;
	config.is_async = false;
	config.use_global_ca_store = false;

	if(m_url == "") return ESP_FAIL;

	if(m_init == false){
		m_init=true;
		ESP_LOGD(TAG,"init connection");
		m_client = esp_http_client_init(&config);
	}else{
		ESP_LOGD(TAG,"re-init connection");
		esp_http_client_close(m_client);
		esp_http_client_cleanup(m_client);
		m_client = esp_http_client_init(&config);
	}
	ESP_LOGD(TAG,"init done");

	if(m_client == NULL) return ESP_FAIL;

	//set methode
	esp_err_t err = esp_http_client_set_method(m_client, method);
	if(err != ESP_OK) return err;

	//add headers
	for(int i= 0; i<m_httpHeadder.size(); i++){
		err = esp_http_client_set_header(m_client, std::get<0>(m_httpHeadder.at(i)).c_str(), std::get<1>(m_httpHeadder.at(i)).c_str());
		if(err != ESP_OK) return err;
		ESP_LOGD(TAG, "Add Headder '%s' with '%s'", std::get<0>(m_httpHeadder.at(i)).c_str(), std::get<1>(m_httpHeadder.at(i)).c_str());
	}

	//set post field
	if(m_postData!="") err = esp_http_client_set_post_field(m_client, m_postData.c_str(), m_postData.length());
	if(err != ESP_OK) return err;

	//perform http request
	ESP_LOGD(TAG,"perform http client..");
	return esp_http_client_perform(m_client);
}

/**
 * @brief      Get http response status code, the valid value if this function invoke after `perform`
 * @return     Status code
 */
int HttpClient::getStatusCode(void){
	return esp_http_client_get_status_code(m_client);
}

/**
 * @brief      Get http response content length (from header Content-Length)
 *             the valid value if this function invoke after `perform`
 * @return
 *     - (-1) Chunked transfer
 *     - Content-Length value as bytes
 */
int HttpClient::getContentLength(void){
	return esp_http_client_get_content_length(m_client);
}

/**
 * @brief      Read data from http stream
 * @param      buffer  The buffer
 * @param[in]  len     The length
 * @return
 *     - (-1) if any errors
 *     - Length of data was read
 */
int HttpClient::getBodyPart(char* buffer, int length){
	return esp_http_client_read(m_client, buffer, length);
}

/**
 * @brief This is the static callback which parses the event in to the specific handler.
 */
esp_err_t HttpClient::event_handler(esp_http_client_event_t *evt){
	HttpClientEventHandler * cb = reinterpret_cast<HttpClientEventHandler*> (evt->user_data);
	if(cb == nullptr) {
		ESP_LOGE(EVENT, "Got an esp_http_client_event_t but no handler found!");
		return ESP_FAIL;
	}
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            cb->onError(evt);
			break;
        case HTTP_EVENT_ON_CONNECTED:
            cb->onConnected(evt);
            break;
        case HTTP_EVENT_HEADER_SENT:
            cb->onHeaderSent(evt);
            break;
        case HTTP_EVENT_ON_HEADER:
            cb->onHeader(evt);
            break;
        case HTTP_EVENT_ON_DATA:
            cb->onData(evt);
            break;
        case HTTP_EVENT_ON_FINISH:
            cb->onFinish(evt);
            break;
        case HTTP_EVENT_DISCONNECTED:
            cb->onDisconnected(evt);
            break;
		default:
			ESP_LOGE(EVENT, "event_id unknown!");
			return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief Class deconstructor.
 *
 */
HttpClient::~HttpClient(){
	esp_http_client_cleanup(m_client);
}

/**
 * @brief Class constructor of the HttpClient event handler.
 * @param [in] your usecase speciffic data pointer.
 */
HttpClientEventHandler::HttpClientEventHandler(void* data):m_data(data){

}

void HttpClientEventHandler::onError(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onError()");
}

void HttpClientEventHandler::onConnected(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onConnected()");
}

void HttpClientEventHandler::onHeaderSent(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onHeaderSent()");
}

void HttpClientEventHandler::onHeader(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onHeader()");
}

void HttpClientEventHandler::onData(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onData()");
}

void HttpClientEventHandler::onFinish(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onFinish()");
}

void HttpClientEventHandler::onDisconnected(esp_http_client_event_t *event){
	ESP_LOGD(EVENT, "default onDisconnected()");
}