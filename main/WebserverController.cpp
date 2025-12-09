/**
 * @brief
 *
 * @file WebserverController.cpp
 * @author your name
 * @date 2018-08-03
 */

#include "Definitions.hpp"
#include "WebserverController.hpp"
#include "FreeRTOS.hpp"
#include "GeneralUtils.hpp"
#include "FileSystem.hpp"

#include <sys/param.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "CallbackWrapper.hpp"

#include <vector>
#include <string>
#include <fstream>



#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"

#include <esp_http_server.h>

static char LOGTAG[] = "WebserverController";

WebserverController::WebserverController(Authenticator& authenticator, CloudController & cloudController):
  m_HttpServer(new HttpServer(16*1024)),
  m_authenticator(authenticator),
  m_cloudController(cloudController),
  m_otaController(new OtaController(m_HttpServer, &m_authenticator)),
  m_jsonRpcCallHandler(nullptr)
{

	m_HttpServer->start();

	std::vector<std::string> dirs;
	dirs.push_back("/");
	dirs.push_back("/assets/*");
	dirs.push_back("/css/*");
	dirs.push_back("/html/*");
	dirs.push_back("/js/*");
	dirs.push_back("/tools/*");
	dirs.push_back("/log/*");
	//dirs.push_back("/_internal/*");
	ESP_ERROR_CHECK(m_HttpServer->startFileServer(std::string(SPIFLASH_PATH), dirs));


	httpd_uri_t rpcCallback = httpd_uri_t();
	rpcCallback.uri       = "/rpc";
    rpcCallback.method    = HTTP_POST;
    rpcCallback.handler   = &WebserverController::rpcHandler;
    rpcCallback.user_ctx  = this; 
	ESP_ERROR_CHECK(m_HttpServer->addPathHandler(rpcCallback));

	httpd_uri_t uploadCallback = httpd_uri_t();
	uploadCallback.uri       = "/upload*";
    uploadCallback.method    = HTTP_POST;
    uploadCallback.handler   = GETCB(HttpServerCallbackHandler, WebserverController)(std::bind(&WebserverController::uploadHandler, this, std::placeholders::_1));
    uploadCallback.user_ctx  = nullptr; 
	ESP_ERROR_CHECK(m_HttpServer->addPathHandler(uploadCallback));

	httpd_uri_t otaCallback = httpd_uri_t();
	otaCallback.uri       = "/ota*";
    otaCallback.method    = HTTP_POST;
    otaCallback.handler   = &WebserverController::otaHandler;
    otaCallback.user_ctx  = this; 
	ESP_ERROR_CHECK(m_HttpServer->addPathHandler(otaCallback));

	httpd_uri_t dataCallback = httpd_uri_t();
	dataCallback.uri       = "/data*";
    dataCallback.method    = HTTP_GET;
    dataCallback.handler   = &WebserverController::dataHandler;
    dataCallback.user_ctx  = this; 
	ESP_ERROR_CHECK(m_HttpServer->addPathHandler(dataCallback));

	httpd_uri_t wsCallback = httpd_uri_t();
	wsCallback.uri       = "/ws";
    wsCallback.method    = HTTP_GET;
    wsCallback.handler   = &WebserverController::wsHandler;
    wsCallback.user_ctx  = this; 
	wsCallback.is_websocket = true;
	ESP_ERROR_CHECK(m_HttpServer->addPathHandler(wsCallback));
}

WebserverController::~WebserverController(){

}

void WebserverController::setJsonRpcCallHandler(JsonRpcCallHandler* jsonRpcCallHandler){
	m_jsonRpcCallHandler = jsonRpcCallHandler;
}

void WebserverController::setExtensionController(ExtensionController* extensionController){
	m_extensionController = extensionController;
}

esp_err_t WebserverController::rpcHandler(httpd_req_t *req) {
	WebserverController *m_handle = reinterpret_cast<WebserverController*>(req->user_ctx);
	httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

	if(m_handle->getJsonRpcCallHandler() != nullptr){

		std::vector<char> buffer (req->content_len,0);

		int ret = httpd_req_recv(req, buffer.data(), buffer.size());
		if (ret <= 0) {  /* 0 return value indicates connection closed */
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
				httpd_resp_send_408(req);
			}
			return ESP_FAIL;
		}
		std::string request  = std::string(buffer.begin(), buffer.end());
		ESP_LOGD(LOGTAG, "RPC Call --> %s", request.c_str());
		std::string response = m_handle->getJsonRpcCallHandler()->parse(request);
		ESP_LOGD(LOGTAG, "RPC Call <-- %s", response.c_str());

    	httpd_resp_send(req, response.c_str(), response.length());
    	return ESP_OK;
	}

	httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "API Error");
    return ESP_FAIL;
}

/**
 * @brief Fileupload to given dir.
 * 
 * @param req http server object
 * @return esp_err_t 
 */
esp_err_t WebserverController::uploadHandler(httpd_req_t *req) {
	//WebserverController *m_handle = reinterpret_cast<WebserverController*>(req->user_ctx);
	httpd_resp_set_hdr(req, "Connection", "close");

	std::string filename = m_HttpServer->getFileName(req);
	std::map<std::string, std::string> qurey = m_HttpServer->getQuery(req); // Get the query part of the request.

	std::string username = "";
	std::string apiToken = "";

	for (auto const& x : qurey){
		if(x.first == "username") username = x.second;
		if(x.first == "token") apiToken = x.second;
		ESP_LOGD(LOGTAG, "Path: %s = %s", x.first.c_str(), x.second.c_str());
    }

	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_ANY);
	if(state != AUTH_STATE_OK){
		httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Not Allowed");
		return ESP_FAIL;
	}
	
    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(LOGTAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than "
                            MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

	//Test placeholder
	if (!FileSystem::isFile(SPIFLASH_PATH INTERNAL_PATH "/upload/"+filename)){
		ESP_LOGE(LOGTAG, "Placeholder for '%s' file not found!", filename.c_str());
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED,
                            "Not Allowed!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
	}

	//create clean file
	std::ofstream file;
	file.open(SPIFLASH_PATH INTERNAL_PATH "/upload/buff", std::fstream::out);
	if (file.is_open()) file << "";
	

    // Retrieve the pointer to scratch buffer for temporary storage 
	std::vector<char> buffer (FILE_UPLOAD_BUFFER_SIZE,0); //reads only the first 1024 bytes
    int received;

    int remaining = req->content_len;

    while (remaining > 0) {

        ESP_LOGI(LOGTAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer 
        if ((received = httpd_req_recv(req, buffer.data(), MIN(remaining, FILE_UPLOAD_BUFFER_SIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred 
                continue;
            }

            // In case of unrecoverable error,
            // close the unfinished file
            file.close();

            ESP_LOGE(LOGTAG, "File reception failed!");
            // Respond with 500 Internal Server Error 
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        // Write buffer content to file on storage 
        file.write (buffer.data(),received);

        // Keep track of remaining size of
        // the file left to be uploaded 
        remaining -= received;
    }

    // Close file upon upload completion 
    file.close();
    ESP_LOGI(LOGTAG, "File reception complete");

	//remove http File Upload Headder from file
	int number_of_lines = 0;
	std::string line;
	std::ifstream infile(SPIFLASH_PATH INTERNAL_PATH "/upload/buff",  std::fstream::binary);

	while (std::getline(infile, line))
		++number_of_lines;

	infile.clear();                 // clear fail and eof bits
	infile.seekg(0, std::ios::beg); // back to the start!

	//erase first 4 and last 2 lines from file
	file.open(SPIFLASH_PATH INTERNAL_PATH "/upload/"+filename, std::fstream::out | std::fstream::binary);
	if (file.is_open()) {
		for(int i = 0; i < number_of_lines; i++){
			std::getline(infile, line);
			if(i>3 && i<number_of_lines-2)
				file << line << std::endl;
		}
	}else ESP_LOGE(LOGTAG, "Unable to open file!");
	file.close();

    // Redirect onto root to see the updated file list 
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}


/**
 * @brief Fileupload to given dir.
 * 
 * @param req http server object
 * @return esp_err_t 
 */
esp_err_t WebserverController::otaHandler(httpd_req_t *req) {
	WebserverController *m_handle = reinterpret_cast<WebserverController*>(req->user_ctx);

	// deactivate cloud Services while update
	m_handle->getCloudController()->stop();

	httpd_resp_set_hdr(req, "Connection", "close");

	std::map<std::string, std::string> qurey = m_handle->getHttpServer()->getQuery(req); // Get the query part of the request.

	std::string username = "";
	std::string apiToken = "";

	for (auto const& x : qurey){
		if(x.first == "username") username = x.second;
		if(x.first == "token") apiToken = x.second;
		ESP_LOGD(LOGTAG, "Path: %s = %s", x.first.c_str(), x.second.c_str());
    }

	auth_state_t state = m_handle->getAuthenticator()->check(username, apiToken, AUTH_LEVEL_ANY);
	if(state != AUTH_STATE_OK){
		httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Not Allowed");
		return ESP_FAIL;
	}

	// Retrieve the pointer to scratch buffer for temporary storage 
	std::vector<char> buffer (FILE_UPLOAD_BUFFER_SIZE,0); //reads only the first 1024 bytes
    int received;

    int remaining = req->content_len;

    while (remaining > 0) {

        ESP_LOGI(LOGTAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer 
        if ((received = httpd_req_recv(req, buffer.data(), MIN(remaining, FILE_UPLOAD_BUFFER_SIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred 
                continue;
            }

            ESP_LOGE(LOGTAG, "File reception failed!");
            // Respond with 500 Internal Server Error 
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        // Write buffer content to file on storage 
        if(!m_handle->getOtaController()->isDone()) 
			m_handle->getOtaController()->performUpdate((uint8_t*)buffer.data(), received, req->content_len);

        // Keep track of remaining size of
        // the file left to be uploaded 
        remaining -= received;
    }

	// Checkout
	ESP_LOGI(LOGTAG, "DONE: %s", m_handle->getOtaController()->getResultString().c_str());
    httpd_resp_sendstr(req, m_handle->getOtaController()->getResultString().c_str());

	// activate cloud Services again
	m_handle->getCloudController()->start();

    return ESP_OK;
}

/**
 * @brief Handler for data Callback from ExtensionController to Graph.js
 * 
 * @param req 
 * @return esp_err_t 
 */
esp_err_t WebserverController::dataHandler(httpd_req_t *req) {
	ESP_LOGI(LOGTAG, "Data Handler");
	WebserverController *m_handle = reinterpret_cast<WebserverController*>(req->user_ctx);
	httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

	if(m_handle->getExtensionController() != nullptr){
		
		m_handle->getExtensionController()->httpDataHandler(req);
    	return ESP_OK;
	}
	
	httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "API Error");
    return ESP_FAIL;
}

/**
 * This handler echos back the received ws data
 * and triggers an async send if certain message received.
 * @param req: The request object
 * @return esp_err_t
 */
esp_err_t WebserverController::wsHandler(httpd_req_t *req) {
    WebserverController *m_handle = reinterpret_cast<WebserverController*>(req->user_ctx);

    //get socket fd and add to vector if not already present
    int fd = httpd_req_to_sockfd(req);
    if (std::find(m_handle->m_websocketClients.begin(), m_handle->m_websocketClients.end(), fd) == m_handle->m_websocketClients.end()){
        ESP_LOGI(LOGTAG, "New WebSocket connection opened");
        m_handle->m_websocketClients.push_back(fd);
    }

    ESP_LOGI("jsonrpc_ws", "Empfange Nachricht von Client-Socket: %d", fd);

    // Handshake done, the new connection was opened 
    if (req->method == HTTP_GET) {
        ESP_LOGI(LOGTAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t)); // Init struct
    
    // Erstes Frame einlesen, um Länge zu erfahren
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(LOGTAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        // remove client from list
        m_handle->m_websocketClients.erase(std::remove(m_handle->m_websocketClients.begin(), m_handle->m_websocketClients.end(), fd), m_handle->m_websocketClients.end());
        return ret;
    }
    
    ESP_LOGD(LOGTAG, "Frame received: length=%d, type=%d", ws_pkt.len, ws_pkt.type);
    
    // Speicher für die Payload bereitstellen
    std::vector<uint8_t> buf;
    if (ws_pkt.len > 0) {
        buf.resize(ws_pkt.len + 1, 0); // Null-terminiert für Texte
        ws_pkt.payload = buf.data();
        
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(LOGTAG, "httpd_ws_recv_frame failed with %d", ret);
            // remove client from list
            m_handle->m_websocketClients.erase(std::remove(m_handle->m_websocketClients.begin(), m_handle->m_websocketClients.end(), fd), m_handle->m_websocketClients.end());
            return ret;
        }
    }

    // Verarbeitung nach Typ der Nachricht

    // Echo für Binärdaten
    if (ws_pkt.type == HTTPD_WS_TYPE_BINARY) {
        ESP_LOGD(LOGTAG, "Binary frame received, echoing back...");
        return httpd_ws_send_frame(req, &ws_pkt); // Echo
    } 

    // Textnachrichten an JSON-RPC-Handler weiterleiten
    else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        std::string request(reinterpret_cast<char*>(ws_pkt.payload));
        ESP_LOGD(LOGTAG, "Text frame received: %s", request.c_str());

        // JSON-RPC Handler aufrufen
        std::string response = m_handle->getJsonRpcCallHandler()->parse(request);
        ESP_LOGD(LOGTAG, "JSON-RPC Response: %s", response.c_str());

        // Antwort als WebSocket-Frame zurücksenden
        httpd_ws_frame_t response_pkt;
        memset(&response_pkt, 0, sizeof(httpd_ws_frame_t));
        response_pkt.type = HTTPD_WS_TYPE_TEXT;
        response_pkt.payload = (uint8_t*)response.c_str();
        response_pkt.len = response.length();

        return httpd_ws_send_frame(req, &response_pkt);
    } 
    
    ESP_LOGW(LOGTAG, "Unsupported WebSocket frame type received.");
    return ESP_OK;
}

/**
 * This function sends a message to all connected WebSocket clients
 * Contenttype is text
 * @param data: The message to send
 */
esp_err_t WebserverController::sendWebsocketBroadcast(std::string data)	{
	return sendWebsocketBroadcastRaw((uint8_t*)data.c_str(), data.length(), HTTPD_WS_TYPE_TEXT);
}

/**
 * This function sends a message to all connected WebSocket clients
 * @param data: The message to send
 * @param len: The length of the message
 * @param type: The type of the message
 */
esp_err_t WebserverController::sendWebsocketBroadcastRaw(uint8_t* data, size_t len, httpd_ws_type_t type) {
    // Check if there are any clients connected
    if (m_websocketClients.empty()) {
        ESP_LOGW("jsonrpc_ws", "Keine aktiven WebSocket-Clients gefunden.");
        return ESP_FAIL;
    }

    // Prepare WebSocket frame
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = type;
    ws_pkt.payload = data;
    ws_pkt.len = len;

    esp_err_t result = ESP_OK;

    // Send message to all clients
    for (int fd : m_websocketClients) {

        ESP_LOGI("jsonrpc_ws", "Sende Nachricht an Client-Socket: %d", fd);
        esp_err_t err = httpd_ws_send_frame_async(m_HttpServer->getServer(), fd, &ws_pkt);

        if (err != ESP_OK) {
            ESP_LOGE("jsonrpc_ws", "Fehler beim Senden an Client %d: %d", fd, err);
            result = err;  // Speichert den letzten Fehler, falls einer auftritt
    
            // remove client from list
            m_websocketClients.erase(std::remove(m_websocketClients.begin(), m_websocketClients.end(), fd), m_websocketClients.end());
        }
    }

    return result;
}