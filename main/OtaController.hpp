/**
 * @file OtaController.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief This class give you three ways to perform an ota update.
 *
 * #1 Update with http Client
 * 		1. Create an instance with your file url:
 * 			OTA otaHttpClient("https://examlple.com/update.bin");
 * 		2. Wait:
 * 			//while(otaHttpClient.isDone) sleep(1);
 * 		3. Check the result:
 * 			if(otaHttpClient.getResult() == "OK")
 * 		4. System::reboot();
 * 		Note: Create a new instance to retry it!
 *
 * #2 Update with http server
 * 		1. Create an instance with a HttpServer instance
 * 			OTA otaHttpServer(m_httpServer);
 * 		2. Call <myIP>/ota with the update file
 * 		3. Check the result:
 * 			if(otaHttpClient.getResult() == "OK")
 * 		4. System::reboot();
 * 		Note: This instance can be used multible times!
 *
 * #3 User defined update
 * 		1. Create an instance
 * 			OTA m_ota;
 * 		2. Garante the start conditions (importent by multible processes)
 * 			m_ota.refreshStartConditions();
 * 		3. write Block by block to the partitions
 * 			m_ota.performUpdate(buffer, packageLength, totalLength);
 * 		4. Check the result:
 * 			if(m_ota.getResult() == "OK")
 * 		5. System::reboot();
 * 		Note: Call m_ota.refreshStartConditions() after m_ota.isDone() to reuse this instance!
 *
 * @version 1.2
 * @date 2019-02-21
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef _OTA_HPP_
#define _OTA_HPP_

#include <string>
#include "esp_ota_ops.h"
#include "Authenticator.hpp"
#include "HttpServer.hpp"
#include "HttpClient.hpp"

#define OTA_APP_PARTITION_PREFFIX 		"app_"
#define OTA_DATA_PARTITION_PREFFIX 		"data_"

//typedef void (* httpPathHandler)(HttpServerRequest *pHttpServerRequest, HttpServerResponse *pHttpServerResponse);

typedef enum {
    OTA_STATE_INIT = 0x0,
    OTA_STATE_PARSE_HEADDER,
    OTA_STATE_WRITE_APP,
	OTA_STATE_ERASE_DATA_PART,
	OTA_STATE_WRITE_DATA,
	OTA_STATE_VALIDATE,
	OTA_STATE_DONE,
	OTA_STATE_ERROR,
	OTA_STATE_MAX,
} ota_state_t;

typedef enum {
	OTA_RESULT_OK = 0x0,
	OTA_RESULT_FILE_NOT_REACHABLE,
	OTA_RESULT_HTTP_GET_ERROR,
	OTA_RESULT_INITIALIZE_ERROR,
	OTA_RESULT_UPDATEFILE_CORRUPTED,
	OTA_RESULT_WRITE_APP_FAILED,
	OTA_RESULT_WRITE_DATA_FAILED,
	OTA_RESULT_VALIDATION_FAILED,
	OTA_RESULT_SAME_FIRMWARE,
	OTA_RESULT_WRONG_FIRMWARE,
	OTA_RESULT_MAX,
} ota_result_t;

class MyHttpClientHander;

/**
 * @brief OtaController performing class.
 */
class OtaController{
public:
	OtaController(); //OtaController update with userdefined protocoll
	OtaController(HttpServer* httpServer, Authenticator* authenticator);
	OtaController(std::string updateURL);
	~OtaController();

	static std::string getCurrentDataPartitionLabel(void); // Needed to mount the right SPIFFS Partition!
	static std::string getUpdateDataPartitionLabel(void);

	void refreshStartConditions(void);
	void performUpdate(uint8_t* buffer, int packageLength, int totalLength);

	bool isDone(void){return m_processDone;}
	std::string getResultString(void){return errorToString(m_result);}
	ota_result_t getResult(void){return m_result;}

	//void httpServerInputHandler(HttpServerRequest *pRequest, HttpServerResponse *pResponse);
	void httpClientInputHandler(int buffersize);

	//only for your self designed update process
	void setError(ota_result_t error);

private:
	//#### FUNCTIONS ####

	//preparation
	void init();
	void start();
	esp_err_t eraseUpdateDataPartition(void);

	//OTA Headder
	ota_result_t parseOtaHeader(std::string part);
	std::string getDataPartitionHash(void);

	//write to partition
	esp_err_t writeAppPartition(uint8_t* data, int length);
	esp_err_t writeDataPartition(uint8_t* data, int length);

	//validating and error
	void validateDataPartition(void);
	void copyDataPartition(void);
	std::string errorToString(ota_result_t error);
	void switchToUpdatePartition(void);
	void debugPartitionInfo();

	

	//#### VARIABLES ####
	// App OTA
	const esp_partition_t *m_bootAppPart;
	const esp_partition_t *m_runningAppPart;
	const esp_partition_t *m_updateAppPart;
	esp_ota_handle_t m_update_app_handle;
	int m_updateAppPartLen{0};
	int m_updateAppPartLeftLen{0};
	uint8_t* m_updateAppPartLeftBytes;

	// Data OTA
	const esp_partition_t *m_runningDataPart;
	const esp_partition_t *m_updateDataPart;
	esp_ota_handle_t m_update_data_handle;
	int m_updateDataPartLen{0};
	int m_readDataPointer{0};
	std::string m_updateDataPartSHA256Hash{""};

	//process variables
	ota_state_t m_state{OTA_STATE_INIT};
	int m_totalLength{0};
	int m_readPointer{0};
	ota_result_t m_result{OTA_RESULT_OK};
	bool m_processDone{false};

	//#### INSTACES ####
	//uses classes
	HttpServer* 			m_httpServer;
	Authenticator*			m_authenticator;
	HttpClient 				m_httpClient;
	friend class 			MyHttpClientHander;
	MyHttpClientHander* 	m_httpClientHandler;
};

// calback for http client

class MyHttpClientHander: public HttpClientEventHandler{
	public:
		MyHttpClientHander(void* data = nullptr);
		~MyHttpClientHander();
		void onConnected(esp_http_client_event_t *event);
		void onData(esp_http_client_event_t *event);
		void onError(esp_http_client_event_t *event);
		void onDisconnected(esp_http_client_event_t *event);
		void onFinish(esp_http_client_event_t *event);

	private:
		OtaController* m_ota;
};

#endif //_OTA_HPP_