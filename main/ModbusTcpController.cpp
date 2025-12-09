/**
 * @file ModbusTcpController.cpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2023-06-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ModbusTcpController.hpp"



static char LOGTAG[]="ModbusTcpController";

ModbusTcpController::ModbusTcpController(CloudController * cloudController, ConfigurationManager & configurationManager, HMIController & hmiController):
	m_param_lock(portMUX_INITIALIZER_UNLOCKED),
	m_cloudController(cloudController),
	m_configurationManager(configurationManager),
	m_hmiController(hmiController){

	ESP_LOGI(LOGTAG, "Starting...");

	//load config from NVS
	uint32_t active, uid, port = 0;
	m_conf.active = false;
	m_conf.uid = 1;
	m_conf.port = 502;


	if (m_configurationManager.getModbusTcpNVS().get("active", active) == ESP_OK)
		m_conf.active = (bool)active;
	if (m_configurationManager.getModbusTcpNVS().get("uid", uid) == ESP_OK)
		m_conf.uid = (uint32_t)uid;
	if (m_configurationManager.getModbusTcpNVS().get("port", port) == ESP_OK)
		m_conf.port = (uint32_t)port;

	if(m_conf.active){
		startServer();
	}else{
		// ESP_ERROR_CHECK(mbc_slave_destroy());
		m_hmiController.setLED3State(HMIController::LEDState::BLINK_SLOW);
	}

}

ModbusTcpController::~ModbusTcpController(void) {

	stopServer();

}

esp_err_t ModbusTcpController::updateHoldingRegisters(std::vector<uint16_t>& updateValues){

	ESP_LOGI(LOGTAG, "Updating holding registers!");
	portENTER_CRITICAL(&m_param_lock);
	for(int i=0; i < MOSBUSTCP_HOLD_REG_NUM; i++){
		m_holding_reg_area[i] = updateValues[i];
	}
	portEXIT_CRITICAL(&m_param_lock);

	return ESP_OK;
}

ModbusTcpConfiguration_t ModbusTcpController::getConfigurationData(void){
	return m_conf;
}

void ModbusTcpController::setConfigurationData(ModbusTcpConfiguration_t newConfig){
	m_conf = newConfig;
	m_configurationManager.getModbusTcpNVS().set("active", (uint32_t)m_conf.active);
	m_configurationManager.getModbusTcpNVS().set("uid", m_conf.uid);
	m_configurationManager.getModbusTcpNVS().set("port", m_conf.port);
	m_configurationManager.getModbusTcpNVS().commit();
	
	//to restart later on
	stopServer();
}

int ModbusTcpController::parseJsonrpcGet(JsonVariant& input, JsonObject& output){

	std::string state;

	if (isRunning()){
		 state = "connected";
	}else{
		state = "disconnected";
	} 

	JsonObject result = output.createNestedObject("result");
	result["select"] = CloudController::MODBUSTCP_SERVER;
	result["active"] = m_conf.active;
	result["state"] = state;
	result["uid"] = m_conf.uid;
	result["port"] = m_conf.port;
	return 0; // means success
}

int ModbusTcpController::parseJsonrpcSet(JsonVariant& input, JsonObject& output){

	ModbusTcpConfiguration_t conf = getConfigurationData();

	JsonVariant active = input["active"];
	JsonVariant uid = input["uid"];
	JsonVariant port = input["port"];

	// these parameters MUST included in the request
	if(!active.isNull()) conf.active = active.as<bool>();
	if(!uid.isNull()) conf.uid = (uint32_t) uid.as<unsigned int>();
	if(!port.isNull()) conf.port = (uint32_t) port.as<unsigned int>();

	setConfigurationData(conf);

	if(conf.active == true){
		startServer();
	}else{
		stopServer();
	}

	return 0;
}

void ModbusTcpController::startServer(){

	m_hmiController.setLED3State(HMIController::LEDState::ON);

	// Initialization of Modbus slave for TCP
	esp_err_t err = mbc_slave_init_tcp(&m_modbusSlaveHandle);
	if (m_modbusSlaveHandle == NULL || err != ESP_OK) {
		// Error handling is performed here
		ESP_LOGE(LOGTAG, "ModbusTcpController initialization fail.");
	}

	//get the esp_netif_t of the Ethernet eth0
	m_netif_eth0 = esp_netif_get_handle_from_ifkey("ETH_SPI_0");
	ESP_LOGI(LOGTAG, "ModbusTcpController working on iface: %s", esp_netif_get_desc(m_netif_eth0));

	//configure the slave stack
	m_comm_info.ip_mode = MB_MODE_TCP;						/*!< Modbus communication mode */
	m_comm_info.slave_uid = m_conf.uid;                     /*!< Modbus slave address field for UID */
	m_comm_info.ip_port = m_conf.port;                    	/*!< Modbus port */
	m_comm_info.ip_addr_type = MB_IPV4;          			/*!< Modbus address type */
	m_comm_info.ip_addr = NULL;                           	/*!< This field keeps the client IP address to bind, NULL - bind to any client */	
	m_comm_info.ip_netif_ptr = m_netif_eth0;      			/*!< Modbus network interface */
			
	// Setup communication parameters and start stack
	ESP_ERROR_CHECK(mbc_slave_setup((void*)&m_comm_info));

	// The code below initializes Modbus register area descriptors
    // for Modbus Holding Registers, Input Registers, Coils and Discrete Inputs
    // Initialization should be done for each supported Modbus register area according to register map.
    // When external master trying to access the register in the area that is not initialized
    // by mbc_slave_set_descriptor() API call then Modbus stack
    // will send exception response for this register area.

	m_reg_area_desc.type = MB_PARAM_HOLDING;                           	// Set type of register area
	m_reg_area_desc.start_offset = 0;             						// Offset of register area in Modbus protocol (first holding register address for this area)
	m_reg_area_desc.address = (void*)&m_holding_reg_area[0];           	// Set pointer to storage instance
	m_reg_area_desc.size = MOSBUSTCP_HOLD_REG_NUM*2;                	// Set the size of register storage area in bytes
	ESP_ERROR_CHECK(mbc_slave_set_descriptor(m_reg_area_desc));

	//start the server
	ESP_ERROR_CHECK(mbc_slave_start());

	isInitialized = true;

}

void ModbusTcpController::stopServer(){
	
	m_hmiController.setLED3State(HMIController::LEDState::BLINK_SLOW);

	if(isInitialized == true){
		ESP_ERROR_CHECK(mbc_slave_destroy());
		isInitialized = false;
	}
}