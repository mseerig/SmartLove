/**
 * @brief
 *
 * @file ExtensionController.cpp
 * @author Marcel Seerig
 * @date 2021-05-12
 */

#include "Definitions.hpp"
#include "System.hpp"
#include "SEN5x.hpp"
#include "ExtensionController.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <math.h>
#include "esp_log.h"
#include "JSON_RPC.hpp"
#include "WS2812.hpp"

static char LOGTAG[] = "ExtensionController";


ExtensionController::ExtensionController(CloudController& cloudController, NetworkController& networkController, ConfigurationManager &configurationManager, HMIController &hmiController, EventLog &eventLog, I2CArbiter &i2cArbiter) : Task("ExtensionController", 2 * 8192, 5),
m_cloudController(cloudController),
m_networkController(networkController),
m_configurationManager(configurationManager),
m_hmiController(hmiController),
m_eventLog(eventLog)
{
	loadDefaultConfig();
	// load config
	std::string buff = "";
	if (m_configurationManager.getExtensionNVS().get("config", &buff) == ESP_OK){

		DeserializationError err = deserializeJson(m_conf, buff);
		if (err != DeserializationError::Ok){
			loadDefaultConfig();
		}

	}else{
		loadDefaultConfig();
	}

	if (m_configurationManager.getFactoryResetFlag())
	{
		// do stuff if required
	}

	//m_airfitController = new AirFitController(m_eventLog, i2cArbiter, m_conf);

	ESP_LOGI(LOGTAG, "Starting");

	Task::start();
}

ExtensionController::~ExtensionController()
{
}

void ExtensionController::loadDefaultConfig(){

	
	m_conf["displayUnits"] = 0;
	m_conf["led_brightness"] = 0.1;
	//...
	
	ESP_LOGI("MAKER", "loadDefaultConfig:");
	std::string conf_str = "";
	serializeJson(m_conf, conf_str);
	ESP_LOGI(LOGTAG, "%s", conf_str.c_str());
	
}

/**
 * @brief loop
 *
 * @param data
 */
void ExtensionController::run(void *data) {
	
	while(1){
		FreeRTOS::sleep(10);
		/*
		if ( m_airfitController->hasNewDataSet() ) {

			std::string content = "";

			serializeJson(m_airfitController->getDataSet(), content);
			ESP_LOGD(LOGTAG, "ESP->CLOUD: %s",content.c_str());
			m_cloudController.sendDeviceDataMessage("", content);

			// TODO add Modbus TCP
			updateModbusTCP();
		}
		*/
	}
}

void ExtensionController::updateModbusTCP(void){
	/*
	DataSet* dataset_now = m_airfitController->getDataSetPointer();
	std::vector<uint16_t> currentHoldReg;

	currentHoldReg.push_back(roundf(dataset_now->getPm1p0()));
	currentHoldReg.push_back(roundf(dataset_now->getPm2p5()));
	currentHoldReg.push_back(roundf(dataset_now->getPm4p0()));
	currentHoldReg.push_back(roundf(dataset_now->getPm10p0()));
	currentHoldReg.push_back(roundf(dataset_now->getVoc()));
	currentHoldReg.push_back(roundf(dataset_now->getNox()));
	currentHoldReg.push_back(roundf(dataset_now->getTemperature()));
	currentHoldReg.push_back(roundf(dataset_now->getHumidity()));
	currentHoldReg.push_back(dataset_now->getStatus());

	m_cloudController.updateBinaryData(currentHoldReg);
	*/
}

/**
 * @brief Incomming MQTT messages will parsed here.
 *
 * @param topic Incomming topic (not used)
 * @param payload Incomming payload as JSON-String
 */
void ExtensionController::parseMQTT(std::string topic, std::string payload)
{
	// ESP_LOGI(LOGTAG, "Topic: %s , Payload: %s", topic.c_str(), payload.c_str());

	std::string content;
	DynamicJsonDocument doc(256);

	if (deserializeJson(doc, payload) == DeserializationError::Ok)
	{

		// doo stuff
	}
	else
	{
		ESP_LOGE(LOGTAG, "Can't parse input data!");
	}
}

/**
 * @brief Handler for incomming JSON-RPC extension.get requests.
 *
 * @param input
 * @param output
 * @return int errorcode
 */
int ExtensionController::parseJsonrpcGet(JsonVariant &input, JsonObject &output){
	JsonObject result = output.createNestedObject("result");
	JsonVariant select = input["select"];

	if(!select.isNull()) {
		std::string whichData = select.as<std::string>();
		if (whichData.compare("data") == 0) {

			//DynamicJsonDocument data = m_airfitController->getDataSet();
			//if(!result.set(data.as<JsonObject>())) return JSONRPC_INTERNAL_ERROR;

			return 0;
		}
		if (whichData.compare("config") == 0) {
			//return whole m_config object
			if(!result.set(m_conf.as<JsonObject>())) return JSONRPC_INTERNAL_ERROR;

			return 0;
		}
		if (whichData.compare("graphsData") == 0) {

			return 0;
		}
		if (whichData.compare("graphsConfig") == 0) {

			return 0;
		}
	}

	return JSONRPC_INVALID_PARAMETER;
}

/**
 * @brief Handler for incomming JSON-RPC extension.set requests.
 *
 * @param input
 * @param output
 * @return int errorcode
 */
int ExtensionController::parseJsonrpcSet(JsonVariant &input, JsonObject &output){

	std::string confStr = "";

	// TODO remove "auth" without creating memory leaks. see https://arduinojson.org/v6/api/jsonobject/remove/
	bool success = m_conf.set(input);
	if(!success) return JSONRPC_INTERNAL_ERROR;

	serializeJson(m_conf.as<JsonObject>(), confStr);
	ESP_LOGD(LOGTAG, "Save: %s", confStr.c_str());
	m_configurationManager.getExtensionNVS().set("config", confStr);
	m_configurationManager.getExtensionNVS().commit();


	//m_eventLog.push(AIRFIT_USER_CHANGED_CONFIG, EventLog::State::INFO);

	//m_airfitController->handleTheasholds();

	return 0;
}

// https://git.ed-chemnitz.de/chemwatch-s/chemwatch-s-firmware/-/branches
/**
 * @brief Extern tiggered reboot preperation routine
 *
 */
void ExtensionController::prepareReboot(void)
{
}

/**
 * @brief send rendom data to cloud
 * 
 * @param chunk_size 
 * @param len 
 * @return uint8_t* 
 */
uint8_t * ExtensionController::getBufferData(uint16_t & len){
	
	/*
	static uint8_t data[DATA_SET_SIZE];
	esp_err_t ret = ESP_FAIL;
	ret = m_airfitController->getBufferData(data, len);
	
	//TODO: Errorhandling
	if(ret != ESP_OK){
		ESP_LOGE(LOGTAG, "Error reading data");
	}

	return data;
	*/
	return nullptr;
}

/**
 * @brief send rendom data to cloud
 * 
 * @param chunk_size 
 * @param len 
 * @return uint8_t* 
 */
uint8_t * ExtensionController::getData(size_t chunk_size, uint16_t & len){
	
	/*
	static uint8_t data[STREAM_CHUNK_SIZE];
	esp_err_t ret = ESP_FAIL;
	
	ret = m_airfitController->getStreamData(data, chunk_size,len);
	
	//TODO: Errorhandling
	if(ret != ESP_OK){
		ESP_LOGE(LOGTAG, "Error reading data");
	}

	return data;
	*/
	return nullptr;
}


/**
 * @brief Handler for data Callback from ExtensionController to Graph.js
 * 
 * @param req 
 * @return esp_err_t 
 */
esp_err_t ExtensionController::httpDataHandler(httpd_req_t *req) {
	httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
		
	/**
	ESP_LOGI(LOGTAG, "sending Data...");

	size_t chunk_size = 1023;
	uint16_t len = 0;

	//get the current data split in a array of given chunksize
	uint8_t * data = getData(chunk_size, len);

	//get Data from Storage
	while (len > 0) {
		// Send the data to the client 
		httpd_resp_send_chunk(req, (const char *)data, len);
		len = 0;
		data = getData(chunk_size, len);
	}

	//get Data from Buffer
	data = getBufferData(len);
	while (len > 0) {
		// Send the data to the client 
		httpd_resp_send_chunk(req, (const char *)data, len);
		len = 0;
		data = getBufferData(len);
	}
	**/

	ESP_LOGI(LOGTAG, "Data sending complete");
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}



/**
 * @brief Extern tiggered reboot preperation routine
 * 
 */
void ExtensionController::factoryReset(void){
	//m_airfitController->eraseFlash();
}