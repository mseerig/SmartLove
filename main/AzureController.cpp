/**
 * @file AzureController.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief
 * @version 0.1
 * @date 2019-11-07
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "AzureController.hpp"

/* Demo Specific configs. */
#include "demo_config.h"


static char LOGTAG[] = "AzureController";

static bool m_disconnectforReconnect = false;

void reconnectTimerCallback(TimerHandle_t pTimer){
    //don´t log here as it will overflow the timer taks buffer size sometimes!!!!!
    // ESP_LOGI(LOGTAG, "Trying to reconnect!");
    m_disconnectforReconnect = false;
}

AzureController::AzureController(CloudController *cloudController, ConfigurationManager &configurationManager, HMIController &hmiController)
	: Task("AzureController", 8192, 5),
	  m_cloudController(cloudController),
	  m_configurationManager(configurationManager),
	  m_hmiController(hmiController){


	ESP_LOGI(LOGTAG, "Starting...");

    m_disconnectforReconnectTimer = xTimerCreate("disconnectforReconnectTimer", 1000, false, nullptr, reconnectTimerCallback);

	m_DeviceID = System::getDeviceID();

	//load config
	uint32_t active = 0;
	m_conf.active = false;

	if (m_configurationManager.getAzureNVS().get("active", active) == ESP_OK)
		m_conf.active = (bool)active;
	if (m_configurationManager.getAzureNVS().get("primConStr", &m_conf.primaryConnectionString) != ESP_OK)
		m_conf.primaryConnectionString = "";

    m_iotHubHostname = getHostname(m_conf.primaryConnectionString);
    m_iotHubDeviceId = getDeviceID(m_conf.primaryConnectionString);
    m_iotHubSasKey = getSasKey(m_conf.primaryConnectionString);

    /* Initialize Azure IoT Middleware.  */
    m_xResult = AzureIoT_Init();
    if(m_xResult != eAzureIoTSuccess){
        ESP_LOGE(LOGTAG, "Initializing the middleware failed: %d", m_xResult);
        //TODO: handle the error
        // return;
    }

    /* Set network credentials including CA cert */
    m_ulStatus = setupNetworkCredentials( &m_xNetworkCredentials );
    if(m_ulStatus != 0){
        ESP_LOGE(LOGTAG, "Setting network credentials failed: %d", m_ulStatus);
        //TODO: handle the error
        // return;
    }

    /* Fill in NetworkContext params */
    m_xNetworkContext.pParams = &m_xTlsTransportParams;

    /* Fill in Transport Interface send and receive function pointers. */
    m_xTransport.pxNetworkContext = &m_xNetworkContext;
    m_xTransport.xSend = TLS_Socket_Send;
    m_xTransport.xRecv = TLS_Socket_Recv;

    /* Init IoT Hub option */
    m_xResult = AzureIoTHubClient_OptionsInit( &m_xHubOptions );
    configASSERT( m_xResult == eAzureIoTSuccess );

	Task::start();
}

AzureController::~AzureController(void){


    //create Semaphore to block until the Task is shut down
    m_taskShutdownDone = xSemaphoreCreateBinary();
    assert(m_taskShutdownDone);

    //signal the task to stop
    m_azureControllerTaskShouldStop = true;

    //wait for the Task to be shut down and hands over the Semaphore
    xSemaphoreTake(m_taskShutdownDone, portMAX_DELAY);
    vSemaphoreDelete(m_taskShutdownDone);

    // AZURE is active, should not
    if(isHubClientConnected()){
        disconnectHubClient();
        deinitHubClient();
        disconnectFromHubServer();
    }

    // AZURE is active, should not: recover from prvious error
    if((!isHubClientConnected()) && (isHubClientInitialized())){
        deinitHubClient();
        disconnectFromHubServer();
    }

    
    // AZURE is active, should not: recover from prvious error
    if((!isHubClientConnected()) && (!isHubClientInitialized()) && (isConnectedToHubServer())){
        disconnectFromHubServer();
    }

	m_hmiController.setLED3State(HMIController::LEDState::OFF);
}

int AzureController::sendDeviceDataMessage(const std::string topic ,const std::string payload){

    if(m_IsHubClientConnected){

         /* Create a bag of properties for the telemetry */
        m_xResult = AzureIoTMessage_PropertiesInit( &m_xPropertyBag, m_ucPropertyBuffer, 0, sizeof( m_ucPropertyBuffer ) );
        if( m_xResult != eAzureIoTSuccess ){
            ESP_LOGE(LOGTAG, "Creating property bag failed: %d", m_xResult);
        }

        /* Append System property */
        JsonObject systemInfo = m_configurationManager.getSystemInfo();
        std::string systemKey = "";
        std::string systemValue = "";

        systemKey = "moduleType";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "name";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "location";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "productVersion";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "deviceID";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "productionDate";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "firmwareVersion";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "hardwareVersion";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "coreVersion";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        systemKey = "upTime";
        systemValue = systemInfo[systemKey].as<std::string>();
        appendPropertyWihtErrorMessage(systemKey, systemValue);

        m_xResult = AzureIoTHubClient_SendTelemetry( &m_xAzureIoTHubClient,
                                                    (const uint8_t*)(payload.c_str()), static_cast<uint32_t>(payload.length()),
                                                    &m_xPropertyBag, eAzureIoTHubMessageQoS1, NULL );
        
        if( m_xResult != eAzureIoTSuccess ){
            ESP_LOGE(LOGTAG, "Sending Telemetry failed: %d", m_xResult);
        }
    }

	return 0;
}

void AzureController::setConfigurationData(AzureConfiguration_t newConfig){
    m_iotHubHostname = getHostname(newConfig.primaryConnectionString);
    m_iotHubDeviceId = getDeviceID(newConfig.primaryConnectionString);
    m_iotHubSasKey = getSasKey(newConfig.primaryConnectionString);
	m_conf = newConfig;

	m_configurationManager.getAzureNVS().set("active", (uint32_t)m_conf.active);
	m_configurationManager.getAzureNVS().set("primConStr", m_conf.primaryConnectionString);
}

AzureConfiguration_t AzureController::getConfigurationData(void){
	return m_conf;
}

void AzureController::parseInputData(const std::string &topic, const std::string &payload){
	// prevent DOS attack!
	if(m_azureData.size() < 10) {
		m_azureData.push_back(make_pair(topic,payload));
		ESP_LOGD(LOGTAG, "Received a MQTT message!");
	}else{
		//send overflow info message
		std::string outStr;
		DynamicJsonDocument doc(1024);
		doc["state"] = "buffer overflow";
		doc["deviceID"] = m_DeviceID;

		TimeConfiguration_t time_conf = m_configurationManager.getTimeConfiguration();
		if(time_conf.enableTimeOutput){
			doc["timestamp"] = System::getLocalTimestamp(time_conf.timezone);
			doc["utcTime"] = (uint32_t) System::getTime();
		}

		serializeJson(doc, outStr);
        //TODO: send this message
		// m_mqtt.publish(m_azure.getPublishTopic(), outStr, 0,  true);
	}
}

void AzureController::handleMqttMessages(){

}

int AzureController::parseJsonrpcGet(JsonVariant& input, JsonObject& output){
	JsonObject result = output.createNestedObject("result");
	result["select"] = CloudController::AZURE;

	std::string  state;
    if(isHubClientConnected()){
        state = "connected";
    }else{
        state = "disconnected";
    }

	result["state"] = state;
	result["active"] = m_conf.active;
	result["hostname"] = getHostname();
	result["deviceID"] = getDeviceID();

	return 0;

}

int AzureController::parseJsonrpcSet(JsonVariant& input, JsonObject& output){
	AzureConfiguration_t conf = getConfigurationData();

	JsonVariant active = input["active"];
	JsonVariant primaryConnectionString = input["primaryConnectionString"];

    // ESP_LOGD(LOGTAG, "PCS is %s", primaryConnectionString.as<std::string>().c_str());

	// these parameters MUST included in the request
    if(active.as<bool>() == false){                                                                             //deactivate Azure

        ESP_LOGD(LOGTAG, "Webfrontend deactivate the Azure Hub Client.");
        conf.active = false;
        setConfigurationData(conf);
        return 0;


    }else if(   (active.as<bool>() == true) 
                && primaryConnectionString.isNull()){                                                           //somebody tries to start with empty PCS

        ESP_LOGW(LOGTAG, "Webfrontend won´t start with empty PCS.");
        return JSONRPC_INVALID_PARAMETER;

    }else if(   (active.as<bool>() == true) 
                && (conf.active == false) 
                && (primaryConnectionString.as<std::string>().compare("xxxxxxxxxxxxxxxx") != 0)){               //client is shut down; start with new pcs

        ESP_LOGD(LOGTAG, "Webfrontend start with new PCS.");
        conf.active =  true;
        conf.primaryConnectionString = primaryConnectionString.as<std::string>();
        setConfigurationData(conf);
        return 0;

    }else if(   (active.as<bool>() == true) 
                && (conf.active == false) 
                && !conf.primaryConnectionString.empty()){                                                      //client is shut down; start with old saved pcs

        ESP_LOGD(LOGTAG, "Webfrontend start with old PCS.");
        conf.active = true;
        setConfigurationData(conf);
        return 0;

    }else if(   (active.as<bool>() == true) 
                && (conf.active == true) 
                && (primaryConnectionString.as<std::string>().compare("xxxxxxxxxxxxxxxx") != 0)){               //client is running; new pcs was entered

        ESP_LOGD(LOGTAG, "Webfrontend new PCS was entered while client is running.");
        conf.active =  true;

        if(conf.primaryConnectionString.compare(primaryConnectionString.as<std::string>()) == 0){                   //old pcs equals new pcs

            ESP_LOGD(LOGTAG, "Webfrontend the new PCS is the same as the old PCS");

        }else{                                                                                                      //old pcs does not equal new pcs
        
            ESP_LOGD(LOGTAG, "Webfrontend the new PCS is not the same as the old PCS");
            conf.primaryConnectionString = primaryConnectionString.as<std::string>();
            
            //create Semaphore to block until the Task is shut down
            m_taskShutdownDone = xSemaphoreCreateBinary();
            assert(m_taskShutdownDone);

            //signal the task to stop
            m_azureControllerTaskShouldStop = true;

            //wait for the Task to be shut down and hands over the Semaphore
            xSemaphoreTake(m_taskShutdownDone, portMAX_DELAY);
            vSemaphoreDelete(m_taskShutdownDone);

            stopClient();

            setConfigurationData(conf);

            m_azureControllerTaskShouldStop = false;

            Task::start();
        }

        
        return 0;

    }else if(   (active.as<bool>() == true) 
                && (conf.active == true) 
                && !conf.primaryConnectionString.empty()){                                                  //client is running; no new pcs was entered

        ESP_LOGD(LOGTAG, "Webfrontend no new PCS was entered. Client is running.");
        return 0;

    }else{                                                                                                  //something went wrong

        return JSONRPC_INVALID_PARAMETER;

    }

}

std::string AzureController::getHostname(std::string primaryConnectionString){
	if(primaryConnectionString=="") primaryConnectionString = m_conf.primaryConnectionString;

	std::string hostname = GeneralUtils::getNextDataItem("HostName=", ";", &primaryConnectionString);
	return hostname;
}

std::string AzureController::getDeviceID(std::string primaryConnectionString){
	if(primaryConnectionString=="") primaryConnectionString = m_conf.primaryConnectionString;

	std::string device_id = GeneralUtils::getNextDataItem("DeviceId=", ";", &primaryConnectionString);

	//erase end
	std::string::size_type endPos = device_id.find(";");
	if(endPos != std::string::npos) device_id.erase(endPos, device_id.length());
	return device_id;
}

std::string AzureController::getSasKey(std::string primaryConnectionString){
    if(primaryConnectionString=="") primaryConnectionString = m_conf.primaryConnectionString;

    std::string keyWord = "SharedAccessKey=";
    std::string sharedAccessKey = "";

    size_t sharedAccessKeyStart = primaryConnectionString.find(keyWord);
    if (sharedAccessKeyStart != std::string::npos) {
        sharedAccessKeyStart += keyWord.size();  // Length of "SharedAccessKey=" is 16
        size_t sharedAccessKeyEnd = primaryConnectionString.size();
        sharedAccessKey = primaryConnectionString.substr(sharedAccessKeyStart, sharedAccessKeyEnd - sharedAccessKeyStart);
    }

    // ESP_LOGI(LOGTAG, "SAS Key parsed: %s", sharedAccessKey.c_str());

    return sharedAccessKey;
}

void AzureController::run(void *data){


	while (m_azureControllerTaskShouldStop == false){
		// set HMI Controller state
		if (isHubClientConnected()) {
			m_hmiController.setLED3State(HMIController::LEDState::ON);
		}else{
			m_hmiController.setLED3State(HMIController::LEDState::BLINK_SLOW);
		}

		FreeRTOS::sleep(10);

        // AZURE is active and connected
        if(isHubClientConnected() && m_conf.active){
			m_xResult = AzureIoTHubClient_ProcessLoop( &m_xAzureIoTHubClient, AzureIotPROCESS_LOOP_TIMEOUT_MS );
            if( m_xResult != eAzureIoTSuccess ){
                ESP_LOGE(LOGTAG, "Failed processing Azure Event loop: %d", m_xResult);
                reconnectDueToExpiredToken();

                //in case the destructor is called because of network loss we still land here first
                //reconnectDueToExpiredToken will shut down the connections but trys to restart after the if is exited
                //in case of destructor we have a race condition against portMaxDelay which we lose
                //with continue we jump back to the while check and exit the task immediately
                ESP_LOGD(LOGTAG, "Jumping one Iteration in case we want to shutdown!");
                continue;
            }
		}

		// AZURE should be active, the client is not connected to the server yet
        if(!isConnectedToHubServer() && m_conf.active && !m_disconnectforReconnect){
			connectToHubServer();
		}

        // AZURE should be active, the client is connected to the server
        if(!isHubClientInitialized() && m_conf.active && !m_disconnectforReconnect){
			initHubClient();
		}

        // AZURE should be active, the client is initialized
        if(!isHubClientConnected() && m_conf.active && !m_disconnectforReconnect){
			connectHubClient();
		}

		// AZURE is active, should not
		if(isHubClientConnected() && (!m_conf.active || m_disconnectforReconnect)){
			disconnectHubClient();
            deinitHubClient();
            disconnectFromHubServer();
		}

        // AZURE is active, should not: recover from prvious error
		if((!isHubClientConnected()) && (isHubClientInitialized()) && (!m_conf.active || m_disconnectforReconnect)){
            deinitHubClient();
            disconnectFromHubServer();
		}

        
        // AZURE is active, should not: recover from prvious error
		if((!isHubClientConnected()) && (!isHubClientInitialized()) && (isConnectedToHubServer())  && (!m_conf.active || m_disconnectforReconnect)){
            disconnectFromHubServer();
		}

	}

    //we got the signal to stop the task and are done. Lets release the semaphore 
    xSemaphoreGive(m_taskShutdownDone);

    ESP_LOGD(LOGTAG, "Task was exited.");
    //stop the task
    Task::stop();
    
    ESP_LOGE(LOGTAG, "Trying to return, we should not have ended up here!");
    return;

}

void AzureController::connectToHubServer(){

    TlsTransportStatus_t xNetworkStatus;

    /* Attempt to connect to IoT Hub. */
    ESP_LOGI( LOGTAG, "Trying to connect to Hub Server %s:%u.\r\n", m_iotHubHostname.c_str(), m_iotHubPort  );

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = TLS_Socket_Connect(    &m_xNetworkContext,
                                            m_iotHubHostname.c_str(), m_iotHubPort,
                                            &m_xNetworkCredentials,
                                            AzureIotTRANSPORT_SEND_RECV_TIMEOUT_MS,
                                            AzureIotTRANSPORT_SEND_RECV_TIMEOUT_MS );


    if( xNetworkStatus != eTLSTransportSuccess){
        ESP_LOGE(LOGTAG, "Was not able establish TLS connection Azure Endpoint: %d", xNetworkStatus);
        m_IsConnectedToHubServer = false;
        abortStartup();
        return;
    }

    m_IsConnectedToHubServer = true;
}

void AzureController::initHubClient(){

    ESP_LOGI(LOGTAG, "Trying to init Hub Client.");

    //Set the Azure Module ID (empty so far)
    m_xHubOptions.pucModuleID = (const uint8_t*)(m_iotHubModuleId.c_str());
    m_xHubOptions.ulModuleIDLength = static_cast<uint32_t>(m_iotHubModuleId.length());


    //Init the client 
    m_xResult = AzureIoTHubClient_Init( &m_xAzureIoTHubClient,
                                        (const uint8_t*)(m_iotHubHostname.c_str()), static_cast<uint32_t>(m_iotHubHostname.length()),
                                        (const uint8_t*)(m_iotHubDeviceId.c_str()), static_cast<uint32_t>(m_iotHubDeviceId.length()),
                                        &m_xHubOptions,
                                        m_ucMQTTMessageBuffer, sizeof( m_ucMQTTMessageBuffer ),
                                        ullGetUnixTime,
                                        &m_xTransport );

    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Azure Hub Client could not initialized: %d", m_xResult);
        m_IsInitialized = false;
        abortStartup();
        return;
    }


    // ESP_LOGD(LOGTAG, "SAS Key is: %s", m_iotHubSasKey.c_str());

    //set the SAS key
    m_xResult = AzureIoTHubClient_SetSymmetricKey(  &m_xAzureIoTHubClient,
                                                    (const uint8_t*)(m_iotHubSasKey.c_str()), 
                                                    static_cast<uint32_t>(m_iotHubSasKey.length()),
                                                    Crypto_HMAC );

    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "SAS Key could not be set: %d", m_xResult);
        m_IsInitialized = false;
        abortStartup();
        return;
    }

    //mark the client as Initialized
    m_IsInitialized = true;
}

void AzureController::connectHubClient(){


    /* Sends an MQTT Connect packet over the already established TLS connection,
    * and waits for connection acknowledgment (CONNACK) packet. */
    ESP_LOGI(LOGTAG, "Trying to create an MQTT connection to %s.", m_iotHubHostname.c_str());

    m_xResult = AzureIoTHubClient_Connect( &m_xAzureIoTHubClient,
                                            false, &m_xSessionPresent,
                                            AzureIotCONNACK_RECV_TIMEOUT_MS );
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Creating an MQTT connection failed: %d", m_xResult);
        m_IsHubClientConnected = false;
        abortStartup();
        return;
    }

    //mark the client as connected
    m_IsHubClientConnected = true;

    // /* Get property document after initial connection */
    // m_xResult = AzureIoTHubClient_RequestPropertiesAsync( &m_xAzureIoTHubClient );
    // if( m_xResult != eAzureIoTSuccess ){
    //     ESP_LOGE(LOGTAG, "Getting property document failed: %d", m_xResult);
    //     abortStartup();
    //     return;
    // }

}

void AzureController::abortStartup(){

    ESP_LOGE(LOGTAG, "Aborting Azure connect.");

    m_disconnectforReconnect = true;

    //TODO: calculate backoff retry value

    xTimerChangePeriod(m_disconnectforReconnectTimer, 1000, portMAX_DELAY);

    ESP_LOGI(LOGTAG, "Setting reconnect timer to %d", xTimerGetPeriod(m_disconnectforReconnectTimer));

    xTimerStart(m_disconnectforReconnectTimer, portMAX_DELAY);


}

void AzureController::stopClient(){

     // AZURE is active, should not
    if(isHubClientConnected()){
        disconnectHubClient();
        deinitHubClient();
        disconnectFromHubServer();
    }

    // AZURE is active, should not: recover from prvious error
    if((!isHubClientConnected()) && (isHubClientInitialized())){
        deinitHubClient();
        disconnectFromHubServer();
    }

    
    // AZURE is active, should not: recover from prvious error
    if((!isHubClientConnected()) && (!isHubClientInitialized()) && (isConnectedToHubServer())){
        disconnectFromHubServer();
    }

}


void AzureController::disconnectFromHubServer(){

    ESP_LOGI(LOGTAG, "Disconnecting from Hub Server.");

    /* Close the network connection.  */
    TLS_Socket_Disconnect( &m_xNetworkContext );

    m_IsConnectedToHubServer = false;

}

void AzureController::deinitHubClient(){
    ESP_LOGI(LOGTAG, "Deinitializing the Hub Client.");

    AzureIoTHubClient_Deinit(&m_xAzureIoTHubClient);

    m_IsInitialized = false;
}

void AzureController::disconnectHubClient(){

    ESP_LOGI(LOGTAG, "Trying to disconnect the Hub Client.");

    m_xResult = AzureIoTHubClient_UnsubscribeProperties( &m_xAzureIoTHubClient );
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Unsubscribing from properties failed: %d", m_xResult);
        return;
    }

    m_xResult = AzureIoTHubClient_UnsubscribeCommand( &m_xAzureIoTHubClient );
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Unsubscribing from command failed: %d", m_xResult);
        return;
    }

    m_xResult = AzureIoTHubClient_UnsubscribeCloudToDeviceMessage( &m_xAzureIoTHubClient );
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Unsubscribing from cloud to device messages failed: %d", m_xResult);
        return;
    }

    /* Send an MQTT Disconnect packet over the already connected TLS over
    * TCP connection. There is no corresponding response for the disconnect
    * packet. After sending disconnect, client must close the network
    * connection. */
    m_xResult = AzureIoTHubClient_Disconnect( &m_xAzureIoTHubClient );
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Sending the MQTT disconnect packet failed: %d", m_xResult);
        return;
    }

    m_IsHubClientConnected = false;

}


void AzureController::reconnectDueToExpiredToken(){
    ESP_LOGI(LOGTAG, "Azure IOT Hub SAS token expired after 1 hour. Peer reset the connection.");

    //Azure closed our socket connection because the SAS key token expired. Lets close everything but don´t use disconnectHubClient

    //overwrite the disconnect
    m_IsHubClientConnected = false;

    //deinit the client
    deinitHubClient();

    //disconnect from server
    disconnectFromHubServer();

    //return to task run. Loop will catch the disco and reinit

}

/**
 * @brief Unix time.
 *
 * @return Time in milliseconds.
 */
uint64_t AzureController::ullGetUnixTime( void )
{
    return System::getTime();
}

/**
 * @brief Setup transport credentials.
 */
uint32_t AzureController::setupNetworkCredentials( NetworkCredentials_t * pxNetworkCredentials )
{
    //TODO: change to our CA cert
    pxNetworkCredentials->xDisableSni = pdFALSE;
    /* Set the credentials for establishing a TLS connection. */
    pxNetworkCredentials->pucRootCa = ( const unsigned char * ) democonfigROOT_CA_PEM;
    pxNetworkCredentials->xRootCaSize = sizeof( democonfigROOT_CA_PEM );
    
    return 0;
}

void AzureController::appendPropertyWihtErrorMessage(std::string key, std::string value){

    if(value == ""){
        value = " ";
    }

    m_xResult = AzureIoTMessage_PropertiesAppend(   &m_xPropertyBag,
                                                    (const uint8_t*)(key.c_str()), static_cast<uint32_t>(key.length()),
                                                    (const uint8_t*)(value.c_str()), static_cast<uint32_t>(value.length()));
    if( m_xResult != eAzureIoTSuccess ){
        ESP_LOGE(LOGTAG, "Appending deviceDataMessage properties failed: %d. Key: %s. Value: %s", m_xResult, key.c_str(), value.c_str());
    }
}




// /**
//  * @brief Cloud message callback handler
//  */
// static void prvHandleCloudMessage( AzureIoTHubClientCloudToDeviceMessageRequest_t * pxMessage,
//                                    void * pvContext )
// {
//     ( void ) pvContext;

//     LogInfo( ( "Cloud message payload : %.*s \r\n",
//                ( int ) pxMessage->ulPayloadLength,
//                ( const char * ) pxMessage->pvMessagePayload ) );
// }
// /*-----------------------------------------------------------*/

// /**
//  * @brief Command message callback handler
//  */
// static void prvHandleCommand( AzureIoTHubClientCommandRequest_t * pxMessage,
//                               void * pvContext )
// {
//     LogInfo( ( "Command payload : %.*s \r\n",
//                ( int ) pxMessage->ulPayloadLength,
//                ( const char * ) pxMessage->pvMessagePayload ) );

//     AzureIoTHubClient_t * xHandle = ( AzureIoTHubClient_t * ) pvContext;

//     if( AzureIoTHubClient_SendCommandResponse( xHandle, pxMessage, 200,
//                                                NULL, 0 ) != eAzureIoTSuccess )
//     {
//         LogInfo( ( "Error sending command response\r\n" ) );
//     }
// }
// /*-----------------------------------------------------------*/

// /**
//  * @brief Property mesage callback handler
//  */
// static void prvHandlePropertiesMessage( AzureIoTHubClientPropertiesResponse_t * pxMessage,
//                                         void * pvContext )
// {
//     ( void ) pvContext;

//     switch( pxMessage->xMessageType )
//     {
//         case eAzureIoTHubPropertiesRequestedMessage:
//             LogInfo( ( "Device property document GET received" ) );
//             break;

//         case eAzureIoTHubPropertiesReportedResponseMessage:
//             LogInfo( ( "Device property reported property response received" ) );
//             break;

//         case eAzureIoTHubPropertiesWritablePropertyMessage:
//             LogInfo( ( "Device property desired property received" ) );
//             break;

//         default:
//             LogError( ( "Unknown property message" ) );
//     }

//     LogInfo( ( "Property document payload : %.*s \r\n",
//                ( int ) pxMessage->ulPayloadLength,
//                ( const char * ) pxMessage->pvMessagePayload ) );
// }
// /*-----------------------------------------------------------*/