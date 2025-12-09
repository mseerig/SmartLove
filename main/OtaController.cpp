/**
 * @file OtaController.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @version 1.2
 * @date 2019-02-21
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "Definitions.hpp"
#include "OtaController.hpp"

#include "HttpServer.hpp"
#include "HttpClient.hpp"
#include "FreeRTOS.hpp"
#include "CallbackWrapper.hpp"
#include "System.hpp"
#include "GeneralUtils.hpp"
#include "Memory.hpp"
#include "SHA256.hpp"
#include "SPIFFS.hpp"
#include "FileSystem.hpp"

#include <esp_log.h>
#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <functional>

static char TAG[] = "OtaController";


/**
 * @brief OtaController constructor.
 */
OtaController::OtaController(HttpServer* httpServer, Authenticator* authenticator):
	m_httpServer(httpServer),
	m_authenticator(authenticator),
	m_httpClientHandler(nullptr)
	{

	init();

	// add the Http request handler to the given httpServer
	//m_httpServer->addPathHandler(
	//	HttpMethod::POST,
	//	"/ota",
	//	GETCB(httpPathHandler, OtaController)(std::bind(&OtaController::httpServerInputHandler, this, std::placeholders::_1,std::placeholders::_2))
	//);
	ESP_LOGD(TAG, "Prepared for update with file-upload!");
}

/**
 * @brief OtaController constructor.
 */
OtaController::OtaController(std::string updateURL):
	m_httpServer(nullptr),
	m_authenticator(nullptr),
	m_httpClientHandler(new MyHttpClientHander(this))
	{
	init();

	//set event handler
	m_httpClient.setEventHandler(m_httpClientHandler);
	m_httpClient.setBuffersize(4096/2);
	m_httpClient.setTimeout(1*60*1000);
	m_httpClient.setUrl(updateURL);

	esp_err_t err = m_httpClient.perform(HTTP_METHOD_GET);
	if(err != ESP_OK)
		setError(OTA_RESULT_FILE_NOT_REACHABLE);
	else
		ESP_LOGI(TAG, "Prepared for http-client update!");

}

/**
 * @brief OtaController constructor.
 */
OtaController::OtaController():
	m_httpServer(nullptr),
	m_authenticator(nullptr),
	m_httpClientHandler(nullptr)
	{

	init();
	ESP_LOGD(TAG, "Prepared for userdefined update!");
}

OtaController::~OtaController(){
	//if(m_httpClientHandler) delete m_httpClientHandler;
	delete m_updateAppPartLeftBytes;
}

/**
 * @brief Init the OtaController class. Reading all necessary information form the partitions.
 */
void OtaController::init(){
	//esp_log_level_set("esp_ota_ops", ESP_LOG_NONE);

	//load App partition info
	m_bootAppPart = esp_ota_get_boot_partition();
	m_runningAppPart = esp_ota_get_running_partition();
	m_updateAppPart = esp_ota_get_next_update_partition(NULL);
	assert(m_updateAppPart != NULL);

	//load Data partition info
	m_runningDataPart = esp_partition_find_first(
		ESP_PARTITION_TYPE_DATA,
		ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
		getCurrentDataPartitionLabel().c_str()
	);
	m_updateDataPart = esp_partition_find_first(
		ESP_PARTITION_TYPE_DATA,
		ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
		getUpdateDataPartitionLabel().c_str()
	);
	assert(m_updateDataPart != NULL);
	m_readDataPointer=0;

	m_updateAppPartLeftBytes = new uint8_t[16];

	debugPartitionInfo();
}

/**
 * @brief Erase all partitions and create update handle.
 * @return error case.
 */
void OtaController::start(){
	//erase the update data Partition
	// ESP_LOGI(TAG, "Erase data_partition now! (this will need a few seconds..)");
	// esp_err_t ret = eraseUpdateDataPartition();
	// if(ret != ESP_OK) {
	// 	ESP_LOGE(TAG, "Erase data_partition failed! %s", GeneralUtils::errorToString(ret));
	// 	setError(OTA_RESULT_INITIALIZE_ERROR); //@ToDo: CHECK!!
	// }

	//beginn with app ota update
	m_update_app_handle = 0;
	esp_err_t ret = esp_ota_begin(m_updateAppPart, OTA_SIZE_UNKNOWN, &m_update_app_handle);
    if (ret != ESP_OK) {
        setError(OTA_RESULT_INITIALIZE_ERROR);
    }else
		ESP_LOGI(TAG, "app: esp_ota_begin succeeded");
}

/**
 * @brief Returns the label of the current data partition.
 * The Funtion is looking for the current boot partition label and parse the partition number from it.
 * Example: The current boot partition is "app_0"
 * -> the current data partition is "data_0"
 * This works only if the right partition preffixes are used in the partitions.csv!
 * @return The partition label, or an empty string, if the "OTA_APP_PARTITION_PREFFIX" of the current boot 		partition is invailed.
 */
std::string OtaController::getCurrentDataPartitionLabel(void){
	const esp_partition_t* bootPart = esp_ota_get_boot_partition();
	std::string dataPart = bootPart->label;
	std::string::size_type preffixStart = dataPart.find(OTA_APP_PARTITION_PREFFIX);
	if(preffixStart != std::string::npos){
		dataPart.replace(0, sizeof(OTA_APP_PARTITION_PREFFIX)-1, OTA_DATA_PARTITION_PREFFIX);
	}else return "";
	return dataPart;
}

/**
 * @brief Returns the label of the data partition for the next firmware update.
 * The Funtion is looking for the current boot partition label and parse the partition number from it.
 * Example: The current boot partition is "app_0"
 * -> update app partition is "app_1"
 * -> update data partition is data_1 !
 * This works only if the right partition preffixes are used in the partitions.csv!
 * @return The partition label, or an empty string, if the "OTA_APP_PARTITION_PREFFIX" of the current boot 		partition is invailed.
 */
std::string OtaController::getUpdateDataPartitionLabel(void){
	const esp_partition_t* bootPart = esp_ota_get_boot_partition();
	std::string dataPart = bootPart->label;
	std::string::size_type preffixStart = dataPart.find(OTA_APP_PARTITION_PREFFIX);
	if(preffixStart != std::string::npos){
		dataPart.erase(0, sizeof(OTA_APP_PARTITION_PREFFIX)-1);
		if(dataPart == "1") dataPart = std::string(OTA_DATA_PARTITION_PREFFIX) + "0";
		else dataPart = std::string(OTA_DATA_PARTITION_PREFFIX) + "1";
	}else return "";
	return dataPart;
}

/**
 * @brief Setting the default values to restart the OtaController process after an error, or success.
 *
 */
void OtaController::refreshStartConditions(void){
	m_state= OTA_STATE_INIT;
	m_processDone = false;
	m_result = OTA_RESULT_OK;
	m_readPointer = 0;
	m_updateAppPartLen = 0; // len of the app partition
	m_readDataPointer = 0;
	m_updateAppPartLeftLen = 0;
	m_updateDataPartLen= 0; // len of the data partition
	m_updateDataPartSHA256Hash = "";
}

/**
 * @brief Starting the Update process, when this handler is triggered by an request on /ota.
 * Returns string. "OK" for success and the errorcase as string, if it's failed.
 *
 * @param pRequest instance of the given http request
 * @param pResponse instance of the following http response
 */
/*
void OtaController::httpServerInputHandler(HttpServerRequest *pRequest, HttpServerResponse *pResponse) {

	std::map<std::string, std::string> qurey = pRequest->getQuery(); // Get the query part of the request.

	std::string username = "";
	std::string apiToken = "";

	for (auto const& x : qurey){
		if(x.first == "username") username = x.second;
		if(x.first == "token") apiToken = x.second;
		ESP_LOGD(TAG, "Path: %s = %s", x.first.c_str(), x.second.c_str());
    }

	auth_state_t state = m_authenticator->check(username, apiToken, AUTH_LEVEL_ANY);
	if(state != AUTH_STATE_OK){
		pResponse->setStatus(HttpStatus::OK, "OK");
		pResponse->addHeader(HttpHeader::CONTENT_TYPE, "text/html");
		pResponse->sendData("Not Allowed");
		pResponse->close();
		return;
	}


	int buffersize = 1024;
	int readPointer = 0;
	int lengthTotal = pRequest->getBodyLength();

	refreshStartConditions();

	uint8_t* buffer = new uint8_t[buffersize];// my data buffer
	while(readPointer < lengthTotal){
		//check the exact rest, by reduse the buffersize
		if(readPointer + buffersize > lengthTotal) buffersize = lengthTotal - readPointer;
		readPointer+= buffersize;
		pRequest->getBodyPart(buffer, buffersize);
		if(!isDone()) performUpdate(buffer, buffersize, lengthTotal);
	}
	delete buffer;

	// Checkout
	// set HTTP response
	pResponse->setStatus(HttpStatus::OK, "OK");
	pResponse->addHeader(HttpHeader::CONTENT_TYPE, "text/plain");
	ESP_LOGI(TAG, "DONE: %s", errorToString(m_result).c_str());
	pResponse->sendData(errorToString(m_result));
	pResponse->close();
	FreeRTOS::sleep(1000);
}*/

/**
 * @brief handles the incomming data from the http client.
 * Calls the process methode with these data.
 *
 * @param buffersize size of the incomming package
 */
void OtaController::httpClientInputHandler(int buffersize){
	//read new block
	uint8_t* buffer = new uint8_t[buffersize];// my data buffer
	m_httpClient.getBodyPart((char*)buffer, buffersize);
	performUpdate(buffer, buffersize, m_httpClient.getContentLength());
	delete buffer;
}

/**
 * @brief Running the update process no matther were the data comes! :)
 * Write package by package with a speciffic length to the partitions.
 * Read the hpp file for detailed information...
 *
 * @param buffer
 * @param packageLength
 */
void OtaController::performUpdate(uint8_t* buffer, int packageLength, int totalLength){
	int BUFFERSIZE = packageLength; //store original buffersize

	// is this the first package?
	if(m_state == OTA_STATE_INIT) {
		start();

		//go to next state
		m_state = OTA_STATE_PARSE_HEADDER;
	}

	if(m_state == OTA_STATE_PARSE_HEADDER){
		//#### STEP 1 #### delete HTTP File Upload Headder (if exists)
		std::string part = std::string((char*)buffer);
		std::string fileHeadderEnd = "\r\n\r\n"; //space between header and content
		std::string::size_type foundHeadderPos = part.find(fileHeadderEnd);

		if(m_httpServer != nullptr){

			ESP_LOGD(TAG, "find upload header...");
			if(foundHeadderPos != std::string::npos){

				m_readPointer = foundHeadderPos + fileHeadderEnd.length(); //set new readPointer
				packageLength -= m_readPointer;

				//move buffer forward
				memcpy( &buffer[0], &buffer[m_readPointer], BUFFERSIZE-m_readPointer * sizeof(uint8_t));
				ESP_LOGV(TAG, "file header erased!");
			}else ESP_LOGI(TAG, "found no file header!");
		}

		//#### STEP 2 #### check the OtaController Headder (must have!)
		std::string otaHeadderEnd = "START:\n"; //space between header and content
		part = std::string((char*)buffer);

		ESP_LOGV(TAG, "find OtaController header...");
		foundHeadderPos = part.find(otaHeadderEnd);
		if(foundHeadderPos != std::string::npos){

			m_readPointer = foundHeadderPos + otaHeadderEnd.length(); //set new readPointer
			packageLength -= m_readPointer;

			//read Partition length from OtaController header
			part.erase(m_readPointer);
			ota_result_t parse_err = parseOtaHeader(part);
			if(parse_err != OTA_RESULT_OK) setError(parse_err);

			//move buffer forward
			memcpy( &buffer[0], &buffer[m_readPointer], BUFFERSIZE-m_readPointer * sizeof(uint8_t));
			ESP_LOGV(TAG, "OtaController header erased!");

		}else {
			setError(OTA_RESULT_UPDATEFILE_CORRUPTED);
		}

		//dose the app+data match in to the left response length? , and no error so faw?
		if(((m_updateAppPartLen + m_updateDataPartLen) <= (totalLength - m_readPointer))){
			if(m_state != OTA_STATE_ERROR){
				m_readPointer = m_updateAppPartLen; //reset to count witten data to app_partition
				m_state = OTA_STATE_WRITE_APP;
			}
		}else setError(OTA_RESULT_UPDATEFILE_CORRUPTED);
	}

	//#### STEP 3 #### Now, write app in partition!
	if(m_state == OTA_STATE_WRITE_APP){
		bool lastPackage = false;
		int readNow = packageLength;

		//set the correct length for the last data package
		if(packageLength >= m_readPointer) {
			readNow = m_readPointer;
			lastPackage = true;
		}

		//reduse left leghts by the current ridden length
		packageLength-= readNow;
		m_readPointer-= readNow;

		//write to partition
		if(writeAppPartition(buffer, readNow) != ESP_OK)
			setError(OTA_RESULT_WRITE_APP_FAILED);
		ESP_LOGD(TAG, "write APP in partition... (%d / %d Bytes)", m_updateAppPartLen-m_readPointer, m_updateAppPartLen);

		//if done, go to next step
		if(lastPackage && m_state != OTA_STATE_ERROR) {
			m_state = OTA_STATE_ERASE_DATA_PART;

			if(packageLength > 0)
				memcpy( &buffer[0], &buffer[BUFFERSIZE-packageLength], packageLength * sizeof(uint8_t));
			m_readPointer = m_updateDataPartLen; //reset to count witten data to data_partition
		}
	}

	//#### STEP 4.1 #### Erase Data partition
	if(m_state == OTA_STATE_ERASE_DATA_PART){
		ESP_LOGI(TAG, "Erase data_partition now! (this will need a few seconds..)");
		esp_err_t ret = eraseUpdateDataPartition();
		if(ret != ESP_OK) {
			ESP_LOGE(TAG, "Erase data_partition failed! %s", GeneralUtils::errorToString(ret));
			setError(OTA_RESULT_WRITE_DATA_FAILED);
		}else{
			m_state = OTA_STATE_WRITE_DATA;
		}
	}

	//#### STEP 4.2 #### Now, write data in partition!
	if(m_state == OTA_STATE_WRITE_DATA){
		bool lastPackage = false;
		int readNow = packageLength;

		//set the correct length for the last data package
		if(packageLength >= m_readPointer) {
			readNow = m_readPointer;
			lastPackage = true;
		}

		//reduse left leghts by the current ridden length
		packageLength-= readNow;
		m_readPointer-= readNow;

		//write to partition
		if(writeDataPartition(buffer, readNow) != ESP_OK)
			setError(OTA_RESULT_WRITE_DATA_FAILED);
		ESP_LOGD(TAG, "write DATA in partition... (%d / %d Bytes)", m_updateDataPartLen-m_readPointer, m_updateDataPartLen);

		//if done, go to next step
		if(lastPackage && m_state != OTA_STATE_ERROR) {
			m_state = OTA_STATE_VALIDATE;
			//#### STEP 5 #### Validate data partition with SHA256 Hash
			validateDataPartition();
			if(m_state == OTA_STATE_ERROR) return;

			copyDataPartition();
			if(m_state == OTA_STATE_ERROR) return;

			//#### STEP 6 #### try to switch the boot partition
			switchToUpdatePartition();
			if(m_state == OTA_STATE_ERROR) return;

			//#### STEP 7 #### Finish
			m_state = OTA_STATE_DONE;
			m_result = OTA_RESULT_OK; //should be "OK", but to be sure..
			m_processDone = true;

			ESP_LOGI(TAG, "The update was an success!");
		}
	}
}

/**
 * @brief Validating the ridden data_partition. This function will "setError()" in
 * case of an error, change the m_state to OTA_STATE_DONE on success.
 *
 */
void OtaController::validateDataPartition(void){
	ESP_LOGV(TAG, "Validate data partition with SHA256 Hash...");
	std::string dataPartitionHash = getDataPartitionHash();
	ESP_LOGV(TAG, "Hash from OtaController header:     %s", m_updateDataPartSHA256Hash.c_str());
	ESP_LOGV(TAG, "Hash from data partition: %s", dataPartitionHash.c_str());

	if(dataPartitionHash != m_updateDataPartSHA256Hash){
		setError(OTA_RESULT_VALIDATION_FAILED);
	}
}

/**
 * @brief copy the content of the
 *
 */
void OtaController::copyDataPartition(void){
	int ret = SPIFFS::mount(OtaController::getUpdateDataPartitionLabel(), "/new");

	if (ret != ESP_OK)	{
		if (ret == ESP_FAIL){
			ESP_LOGE(TAG, "Failed to mount /new filesystem");
			setError(OTA_RESULT_WRITE_DATA_FAILED);
			return;
		}else if (ret == ESP_ERR_NOT_FOUND)		{
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
			setError(OTA_RESULT_WRITE_DATA_FAILED);
			return;
		}else{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
			setError(OTA_RESULT_WRITE_DATA_FAILED);
			return;
		}
	}


    FileSystem::copy(SPIFLASH_PATH "/_internal", "/new/_internal");
}

/**
 * @brief Trying to swich the boot partition to the new updated partition.
 * @return string. Error case in a user friendly text.
 */
void OtaController::switchToUpdatePartition(void){

    if (esp_partition_check_identity(esp_ota_get_running_partition(), m_updateAppPart) == true) {
		setError(OTA_RESULT_SAME_FIRMWARE);
		return;
    }

    esp_err_t err = esp_ota_set_boot_partition(m_updateAppPart);
    if (err != ESP_OK) {
        setError(OTA_RESULT_VALIDATION_FAILED);
		return;
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
}

/*
* @brief Find the OtaController header created by "esp32-ota-merge/OTA_MERGE.py" and parse it
* in to the m_updateAppPartLen and m_updateDataPartLen variables.
* @param [in] std::string wich contains the OtaController header.
* @return error case (not found), or ok.
*/
ota_result_t OtaController::parseOtaHeader(std::string part){
	//#### STEP 2.0 #### get MODULE_TYPE
	std::string moduleType = GeneralUtils::getNextDataItem("<", ">", &part);
	if(moduleType != ""){
		ESP_LOGD(TAG, "Module Type: %s ", moduleType.c_str());
		if(moduleType != MODULE_TYPE) return OTA_RESULT_WRONG_FIRMWARE;
	}else return OTA_RESULT_UPDATEFILE_CORRUPTED;

	//#### STEP 2.1 #### get APP len
	std::string app_len_str = GeneralUtils::getNextDataItem("<", ">", &part);
	if(app_len_str != ""){
		m_updateAppPartLen = atoi(app_len_str.c_str());
		ESP_LOGD(TAG, "APP Partition length: %d Bytes", m_updateAppPartLen);
	}else return OTA_RESULT_UPDATEFILE_CORRUPTED;

	//#### STEP 2.2 #### get DATA len
	std::string data_len_str = GeneralUtils::getNextDataItem("<", ">", &part);
	if(data_len_str != ""){
		m_updateDataPartLen = atoi(data_len_str.c_str());
		ESP_LOGD(TAG, "DATA Partition length: %d Bytes", m_updateDataPartLen);
	}else return OTA_RESULT_UPDATEFILE_CORRUPTED;

	//#### STEP 2.3 #### get DATA SHA256 m_updateDataPartSHA256Hash
	m_updateDataPartSHA256Hash = GeneralUtils::getNextDataItem("<", ">", &part);
	if(m_updateDataPartSHA256Hash != ""){
		ESP_LOGD(TAG, "DATA Partition Hash: %s", m_updateDataPartSHA256Hash.c_str());
	}else return OTA_RESULT_UPDATEFILE_CORRUPTED;

	return OTA_RESULT_OK;
}

/**
 * @brief Write OtaController Data to the app partition.
 * @param [in] given datablock.
 * @param [in] given datablock length.
 * @return the esp error code, or ESP_OK
 */
esp_err_t OtaController::writeAppPartition(uint8_t* data, int length){

	esp_err_t err;
	int locallength = length;
	int bytestofill = 0;
	int writeablelength = 0;

	//habe ich Pakete die ich beim letzten mal nicht schreiben konnte?
	//wenn ja: fülle ich meinen rest auf 16 bytes auf und schreibe die weg
	if (m_updateAppPartLeftLen != 0 && m_updateAppPartLeftLen < 16)
	{
		//## Schritt 1:
		// den Rest vom letzten schreiben nutzen und mit neuen Bytes auf eine länge
		// von 16 Bytes vergrößern um im 16er-Block schreiben zu können.
		ESP_LOGI(TAG, "ich habe einen rest %d", m_updateAppPartLeftLen);
		bytestofill = 16-m_updateAppPartLeftLen; // length of fill
		locallength -= bytestofill; // rest = rest - fill
		memcpy(&m_updateAppPartLeftBytes[m_updateAppPartLeftLen],&data[0],bytestofill*sizeof(uint8_t));
		err = esp_ota_write(m_update_app_handle, (const void *)m_updateAppPartLeftBytes, 16);
		if(err != ESP_OK) return err;

		//## Schritt 2:
		// Schreibe einen größtmöglichen bock der von ler länge durch 16 teilbar ist.
		writeablelength = (locallength / 16) *16; // ganzzahlinges Ergebnis
		m_updateAppPartLeftLen = locallength %16;
		memcpy( &data[0], &data[bytestofill], locallength * sizeof(uint8_t));
		err = esp_ota_write(m_update_app_handle, (const void *)data, writeablelength);
		if(err != ESP_OK) return err;

		//## Schritt 3:
		// Kopiere den Rest in den Buffer um diesen das nächste mal zu schreiben
		memcpy(&m_updateAppPartLeftBytes[0],&data[writeablelength],(m_updateAppPartLeftLen)*sizeof(uint8_t));
	}
	else if (m_updateAppPartLeftLen == 0){
		//## Schritt 1:
		// Schreibe einen größtmöglichen bock der von ler länge durch 16 teilbar ist.
		ESP_LOGI(TAG, "ich habe kein rest");
		writeablelength = (length / 16)*16;
		m_updateAppPartLeftLen = length %16;
		err = esp_ota_write(m_update_app_handle, (const void *)data, writeablelength);
		if(err != ESP_OK) return err;

		//## Schritt 2:
		// Kopiere den Rest in den Buffer um diesen das nächste mal zu schreiben
		memcpy(&m_updateAppPartLeftBytes[0],&data[writeablelength],(m_updateAppPartLeftLen)*sizeof(uint8_t));
	}
	else {
		return ESP_ERR_INVALID_SIZE;
	}

	// Handelt es sich um das letzte packet? gibt es einen rest?
	if (m_readPointer == 0 && m_updateAppPartLeftLen != 0){
		ESP_LOGI(TAG, "ich habe ein rest '%d' im letzten Packet!", m_updateAppPartLeftLen);

		// now fill the rest with zeros to have valid 16bytes
		for(int i = m_updateAppPartLeftLen; i<16; i++) m_updateAppPartLeftBytes[i] = 0x00;

		err = esp_ota_write(m_update_app_handle, (const void *)m_updateAppPartLeftBytes, 16);
		if(err != ESP_OK) return err;
	}

	return err;
}

/**
 * @brief Write OtaController Data to the data partition.
 * @param [in] given datablock.
 * @param [in] given datablock length.
 * @return the esp error code, or ESP_OK
 */
esp_err_t OtaController::writeDataPartition(uint8_t* data, int length){
	esp_err_t err = esp_partition_write(m_updateDataPart, m_readDataPointer, (const void *)data, length);
	m_readDataPointer+=length;
	return err;
}

/**
 * @brief Erase the following OtaController Data partition for a re-write.
 * @param N/A.
 * @return the esp error code, or ESP_OK
 */
esp_err_t OtaController::eraseUpdateDataPartition(void){
	return esp_partition_erase_range(m_updateDataPart, 0x00, m_updateDataPart->size);
}

/**
 * @brief Generate the SHA256 Hash from the Data partition
 * @param N/A
 * @return hash code, or empty string
 */
std::string OtaController::getDataPartitionHash(void){

	esp_err_t err = ESP_OK;
	SHA256 sha;

	int BUFFERSIZE = 1024;
	uint8_t* buffer = (uint8_t*) malloc((size_t) BUFFERSIZE); // my data buffer

	int offset = 0;
	int readSize = BUFFERSIZE;
	while(err == ESP_OK && offset < m_updateDataPart->size){

		if(m_updateDataPart->size - offset < BUFFERSIZE) // read exact the rest
			readSize = m_updateDataPart->size - offset;

		err = esp_partition_read(m_updateDataPart, offset, buffer, readSize); //read to buffer
		sha.update((unsigned char*)buffer, readSize); // this API cause SIGBUS

		if(err != ESP_OK) ESP_LOGD(TAG, "Error while calculating SHA256 hash!");

		offset += readSize;
	}
	free(buffer);

	return sha.getResultHash();
}

/**
 * @brief In case of an error in some internal routines, this function is called to
 * set the m_state to OTA_STATE_ERROR, print the error to the log output and will
 * set the m_processDone to true, that a new update process can be started.
 *
 * @param errorText error message (printed also in the GUI!)
 */
void OtaController::setError(ota_result_t error){
	m_result = error;
	ESP_LOGE(TAG, "GOT ERROR: %s", errorToString(m_result).c_str());
	m_state = OTA_STATE_ERROR;
	m_processDone = true;
}

/**
 * @brief converting the error code to string.
 *
 * @param error ota_result_t
 * @return std::string error text
 */
std::string OtaController::errorToString(ota_result_t error){
	switch (error) {
		case OTA_RESULT_OK:
			return "OK";
		case OTA_RESULT_FILE_NOT_REACHABLE:
			return "The file is not reachable, check your link!";
		case OTA_RESULT_HTTP_GET_ERROR:
			return "Error while downloading file!";
		case OTA_RESULT_INITIALIZE_ERROR:
			return "Update can't be initialized!";
		case OTA_RESULT_UPDATEFILE_CORRUPTED:
			return "Your update file is corrupted!";
		case OTA_RESULT_WRITE_APP_FAILED:
			return "Write to app partition failed!";
		case OTA_RESULT_WRITE_DATA_FAILED:
			return "Write to data partition failed!";
		case OTA_RESULT_VALIDATION_FAILED:
			return "Validating the written firmware failed!";
		case OTA_RESULT_SAME_FIRMWARE:
			return "This was the same firmware like befor.";
		case OTA_RESULT_WRONG_FIRMWARE:
			return "The given Firmware is not designed for this Hardware!";
		default: return "Unknown error.";
	}
}

/**
 * @brief Debug some Partition information.
 */
void OtaController::debugPartitionInfo(){
	// debug app partition info
	if (m_bootAppPart != m_runningAppPart) {
        ESP_LOGW(TAG, "App: Configured OtaController boot partition at offset 0x%08x, but running from offset 0x%08x",
                 m_bootAppPart->address, m_bootAppPart->address);
        ESP_LOGW(TAG, "App: (This can happen if either the OtaController boot data or preferred boot image become corrupted somehow.)");
	}
	ESP_LOGI(TAG, "App: Running partition type %d subtype %d (offset 0x%08x)",
			m_runningAppPart->type, m_runningAppPart->subtype, m_runningAppPart->address);
    ESP_LOGI(TAG, "App: Writing partition type %d subtype %d (offset 0x%08x)",
        	m_updateAppPart->type, m_updateAppPart->subtype, m_updateAppPart->address);

	// debug data partition info
	ESP_LOGI(TAG, "Data: Running partition type %d subtype %d (offset 0x%08x)",
			m_runningDataPart->type, m_runningDataPart->subtype, m_runningDataPart->address);
    ESP_LOGI(TAG, "Data: Writing partition type %d subtype %d (offset 0x%08x)",
        	m_updateDataPart->type, m_updateDataPart->subtype, m_updateDataPart->address);
}


MyHttpClientHander::MyHttpClientHander(void* data){
	m_ota = reinterpret_cast<OtaController*>(data);
}

void MyHttpClientHander::onConnected(esp_http_client_event_t *event){
	ESP_LOGD(TAG, "HttpClient onConnected()");
	//m_ota->refreshStartConditions();
}

void MyHttpClientHander::onData(esp_http_client_event_t *event){
	m_ota->httpClientInputHandler(event->data_len);
}

void MyHttpClientHander::onDisconnected(esp_http_client_event_t *event){
	ESP_LOGD(TAG, "HttpClient onDisconnected()");
	if(!m_ota->isDone()){
		m_ota->setError(OTA_RESULT_HTTP_GET_ERROR);
	}
}

void MyHttpClientHander::onError(esp_http_client_event_t *event){
	ESP_LOGD(TAG, "HttpClient onError()");
	m_ota->setError(OTA_RESULT_HTTP_GET_ERROR);
}

void MyHttpClientHander::onFinish(esp_http_client_event_t *event){
	ESP_LOGD(TAG, "HttpClient onFinish()");
	if(!m_ota->isDone()){
		m_ota->setError(OTA_RESULT_HTTP_GET_ERROR);
	}
}

MyHttpClientHander::~MyHttpClientHander(){

}