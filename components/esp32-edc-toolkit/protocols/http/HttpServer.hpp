/**
 * @file HttpServer.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version v0.0.1
 * @date 2021-01-18
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef COMPONENTS_CPP_UTILS_HTTPSERVER_H_
#define COMPONENTS_CPP_UTILS_HTTPSERVER_H_

#include <map>
#include <vector>
#include <string>

#include "esp_err.h"
#include "esp_http_server.h"

typedef esp_err_t (HttpServerCallbackHandler)(httpd_req_t *req);
class HttpServer {
public:
	HttpServer(size_t stack_size = (8*1024));
	~HttpServer();

	esp_err_t       	start();
	esp_err_t     		stop();
	esp_err_t 			addPathHandler(httpd_uri_t handler);
	std::string			getRootDir(){return m_rootdir;}
	void				setRootDir(std::string rootdir);
	esp_err_t 			startFileServer(std::string rootdir, std::vector<std::string>dirs);

	std::map<std::string, std::string> getQuery(httpd_req_t *req);
	std::string 		getFileName(httpd_req_t *req);

	std::vector<int> 	getClientSockets();

	static esp_err_t	default_handler(httpd_req_t *req);

	httpd_handle_t 		getServer(){return m_server;}
	
private:

	esp_err_t 			setContentType(httpd_req_t *req, std::string filename);

	httpd_handle_t 		m_server;
    httpd_config_t 		m_config;
	std::string 		m_rootdir;
}; // HttpServer

#endif /* COMPONENTS_CPP_UTILS_HTTPSERVER_H_ */
