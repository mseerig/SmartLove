/**
 * @file HttpServer.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2021-01-18
 *
 * @copyright Copyright (c) 2021
 *
 */


// https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/file_serving/main


#include <cstdio>
#include <string>
#include <map>
#include <fstream>

#include "esp_http_server.h"
#include "esp_vfs.h" //for max file size
#include <esp_log.h>
#include <esp_err.h>

#include "HttpServer.hpp"
#include "CallbackWrapper.hpp"
#include "GeneralUtils.hpp"
#include "FileSystem.hpp"
#include "Task.hpp"


#define HTTP_RESPONSE_BUFSIZE  8192
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

static const char* TAG = "HttpServer";

/**
 * Constructor for HTTP Server
 */
HttpServer::HttpServer(size_t stack_size) {
	m_server = NULL;
    m_config = HTTPD_DEFAULT_CONFIG();
	m_config.max_uri_handlers = 16;
	m_config.stack_size = stack_size;
	m_rootdir= "/root";
} // HttpServer


HttpServer::~HttpServer() {
	ESP_LOGI(TAG, "~HttpServer");
}

esp_err_t HttpServer::start() {
    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    m_config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&m_server, &m_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    /* URI handler for getting uploaded files */
    return ESP_OK;
} // start

esp_err_t HttpServer::startFileServer(std::string rootdir, std::vector<std::string>dirs){
    setRootDir(rootdir);

    httpd_uri_t defaultHandler = httpd_uri_t();
	defaultHandler.uri       = "";
    defaultHandler.method    = HTTP_GET;
    defaultHandler.handler   = &HttpServer::default_handler;
    defaultHandler.user_ctx  = this; 

    for(const auto& dir: dirs){
        defaultHandler.uri = dir.c_str();
        esp_err_t err = addPathHandler(defaultHandler);
        if(err != ESP_OK) return err;
    }
    
    return ESP_OK;
}

esp_err_t HttpServer::stop() {
    
    if (httpd_stop(&m_server) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop file server!");
        return ESP_FAIL;
    }
    return ESP_OK;
} // stop

esp_err_t HttpServer::addPathHandler(httpd_uri_t handler){
    return httpd_register_uri_handler(m_server, &handler);
}

#define STATE_NAME  0
#define STATE_VALUE 1

/**
 * @brief Get the query part of the request.
 * The query is a set of name = value pairs.  The return is a map keyed by the name items.
 *
 * @return The query part of the request.
 */
std::map<std::string, std::string> HttpServer::getQuery(httpd_req_t *req) {
	// Walk through all the characters in the query string maintaining a simple state machine
	// that lets us know what we are parsing.
	std::map<std::string, std::string> queryMap;

	std::string possibleQueryString = std::string(req->uri);
	int qindex = possibleQueryString.find_first_of("?") ;
	if (qindex < 0) {
		//ESP_LOGD(LOG_TAG, "No query string present") ;
		return queryMap ;
	}
	std::string queryString = possibleQueryString.substr(qindex + 1, -1) ;
	//ESP_LOGD(LOG_TAG, "query string: %s", queryString.c_str()) ;

	/*
	 * We maintain a simple state machine with states of:
	 * * STATE_NAME - We are parsing a name.
	 * * STATE_VALUE - We are parsing a value.
	 */
	int state = STATE_NAME;
	std::string name = "";
	std::string value;
	// Loop through each character in the query string.
	for (int i=0; i<queryString.length(); i++) {
		char currentChar = queryString[i];
		if (state == STATE_NAME) {
			if (currentChar != '=') {
				name += currentChar;
			} else {
				state = STATE_VALUE;
				value = "";
			}
		} // End state = STATE_NAME
		else if (state == STATE_VALUE) {
			if (currentChar != '&') {
				value += currentChar;
			} else {
				////ESP_LOGD(tag, "name=%s, value=%s", name.c_str(), value.c_str());
				queryMap[name] = value;
				state = STATE_NAME;
				name = "";
			}
		} // End state = STATE_VALUE
	} // End for loop
	if (state == STATE_VALUE) {
		////ESP_LOGD(tag, "name=%s, value=%s", name.c_str(), value.c_str());
		queryMap[name] = value;
	}
	return queryMap;
} // getQuery

/* Set HTTP response content type according to file extension */
esp_err_t HttpServer::setContentType(httpd_req_t *req, std::string filename){
	if(GeneralUtils::endsWith(filename, ".pdf" )) return httpd_resp_set_type(req, "application/pdf");
    if(GeneralUtils::endsWith(filename, ".html")) return httpd_resp_set_type(req, "text/html");
    if(GeneralUtils::endsWith(filename, ".js"  )) return httpd_resp_set_type(req, "text/javascript");
    if(GeneralUtils::endsWith(filename, ".css" )) return httpd_resp_set_type(req, "text/css");
    if(GeneralUtils::endsWith(filename, ".jpeg")) return httpd_resp_set_type(req, "image/jpeg");
    if(GeneralUtils::endsWith(filename, ".ico" )) return httpd_resp_set_type(req, "image/x-icon");
	if(GeneralUtils::endsWith(filename, ".svg" )) return httpd_resp_set_type(req, "image/svg+xml");

    /* This is a limited set only */
    /* For any other type always set as plain text */
   return httpd_resp_set_type(req, "text/plain");
}

void HttpServer::setRootDir(std::string rootdir){
	m_rootdir = rootdir;
	if(GeneralUtils::endsWith(m_rootdir, '/')){
		m_rootdir.erase(m_rootdir.end());
	}
}

std::string HttpServer::getFileName(httpd_req_t *req){

	std::string filename = GeneralUtils::split(std::string(req->uri), '/').back(); //kepp back of '/'
	if (filename.find('?') != std::string::npos)
		filename = GeneralUtils::split(filename, '?').front(); //keep front of '?'
	return filename;
}

/* Handler to download a file kept on the server */
esp_err_t HttpServer::default_handler(httpd_req_t *req){
	HttpServer *m_handle = reinterpret_cast<HttpServer*>(req->user_ctx);

	std::string rootPath = "/spiflash";

	std::string filepath = rootPath + std::string(req->uri);
	std::string filename = m_handle->getFileName(req);

	if(std::string(req->uri) == "/"){
		filename = "index.html" ;
		filepath = rootPath + "/index.html";
	}

    if (filepath.length() > FILE_PATH_MAX) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    if (!FileSystem::isFile(filepath)) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath.c_str());
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    m_handle->setContentType(req, filename.c_str());

	std::ifstream file;
	file.open(filepath, std::ifstream::binary);
	if(!file.is_open()){
		ESP_LOGE(TAG, "Failed to read existing file : %s", filepath.c_str());
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
	}

	std::vector<char> buffer (HTTP_RESPONSE_BUFSIZE,0); //reads only the first 1024 bytes

	while(!file.eof()) {
		file.read(buffer.data(), buffer.size());
		std::streamsize s=file.gcount();

		if (httpd_resp_send_chunk(req, buffer.data(), s) != ESP_OK) {
			file.close();
			ESP_LOGE(TAG, "File sending failed!");
			/* Abort sending file */
			httpd_resp_sendstr_chunk(req, NULL);
			/* Respond with 500 Internal Server Error */
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
			return ESP_FAIL;
		}
	}

    /* Close file after sending complete */
    file.close();
    //ESP_LOGI(TAG, "File sending complete");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* Gibt eine Liste mit den Socket ID zurück, welche aktuell mit dem ESP verbunden sind.*/
std::vector<int> HttpServer::getClientSockets() {
    size_t max_clients = CONFIG_LWIP_MAX_SOCKETS;  // Maximale Anzahl an Clients
    std::vector<int> client_fds(max_clients);  // Vektor mit maximal möglicher Anzahl initialisieren

    esp_err_t err = httpd_get_client_list(m_server, &max_clients, client_fds.data());
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Fehler beim Abrufen der Client-Liste: %d", err);
        return {}; // Leerer Vektor zurückgeben
    }

    // Reduziere die Größe des Vektors auf die tatsächliche Anzahl der Clients
    client_fds.resize(max_clients);

    return client_fds;
}