/**
 * @brief
 *
 * @file WebserverController.hpp
 * @author your name
 * @date 2018-08-03
 */

#ifndef WEBSERVER_CONTROLLER_H_
#define WEBSERVER_CONTROLLER_H_

#include "Authenticator.hpp"
#include "JsonRpcCallHandler.hpp"
#include "OtaController.hpp"

#include "HttpServer.hpp"
#include "JSON_RPC.hpp"

#include <map>

#define MAX_FILE_SIZE   			(200*1024) // 200 KB
#define MAX_FILE_SIZE_STR 			"200KB"
#define FILE_UPLOAD_BUFFER_SIZE 	8192
#define STREAM_CHUNK_SIZE 			1023

class JsonRpcCallHandler;

class WebserverController {
public:
	WebserverController(Authenticator& authenticator, CloudController& cloudController);
	~WebserverController(void);

	void start();
	void stop();


	static esp_err_t rpcHandler(httpd_req_t *req);
	esp_err_t uploadHandler(httpd_req_t *req);
	static esp_err_t otaHandler(httpd_req_t *req);
	static esp_err_t dataHandler(httpd_req_t *req);
	static esp_err_t wsHandler(httpd_req_t *req);
	
	esp_err_t sendWebsocketBroadcast(std::string data);	
	esp_err_t sendWebsocketBroadcastRaw(uint8_t* data, size_t len, httpd_ws_type_t type);

	void setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler);
	void setExtensionController(ExtensionController* extensionController);

	HttpServer* getHttpServer(){return m_HttpServer;}
	Authenticator* getAuthenticator(){return &m_authenticator;}
	CloudController* getCloudController(){return &m_cloudController;}
	OtaController* getOtaController(){return m_otaController;}
	JsonRpcCallHandler* getJsonRpcCallHandler(){return m_jsonRpcCallHandler;};
	ExtensionController* getExtensionController(){return m_extensionController;}

private:

	HttpServer*				m_HttpServer;
	Authenticator&			m_authenticator;
	CloudController&		m_cloudController;
	OtaController*			m_otaController;
	JsonRpcCallHandler*  	m_jsonRpcCallHandler;
	ExtensionController*	m_extensionController;
	
public:
	std::vector<int> m_websocketClients{CONFIG_LWIP_MAX_SOCKETS};
};

#endif