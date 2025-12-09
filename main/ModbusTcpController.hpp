/**
 * @file ModbusTcpController.hpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2023-06-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#ifndef MODBUS_TCP_CONTROLLER_H_
#define MODBUS_TCP_CONTROLLER_H_

#include "Definitions.hpp"
#include "NetworkController.hpp"
#include "ConfigurationManager.hpp"

#include "FreeRTOS.hpp"

#include "mbcontroller.h"
#include "esp_netif.h"
#include "esp_log.h"

#include <vector>


typedef struct {
	bool active;
	uint32_t uid;
	uint32_t port;
}ModbusTcpConfiguration_t;

class CloudController;
class HMIController;
class NetworkController;

class ModbusTcpController{
    public:
        ModbusTcpController(CloudController *cloudController, ConfigurationManager &configurationManager, HMIController &hmiController);
        ~ModbusTcpController();

        esp_err_t updateHoldingRegisters(std::vector<uint16_t>& updateValues);

        ModbusTcpConfiguration_t getConfigurationData(void);
        void setConfigurationData(ModbusTcpConfiguration_t newConfig);

        int parseJsonrpcGet(JsonVariant& input, JsonObject& output);
	    int parseJsonrpcSet(JsonVariant& input, JsonObject& output);

        bool isRunning(){return isInitialized;}


    private:

        void                            startServer();
        void                            stopServer();
        bool                            isInitialized{false};

        void *                          m_modbusSlaveHandle;
	    mb_communication_info_t         m_comm_info{.ip_mode=MB_MODE_TCP};                  //one initializer needed to get in the right part of the uion
        mb_register_area_descriptor_t   m_reg_area_desc; 						            // Modbus register area descriptor structure
        uint16_t                        m_holding_reg_area[MOSBUSTCP_HOLD_REG_NUM]{0};
        portMUX_TYPE                    m_param_lock;
        esp_netif_t *                   m_netif_eth0;


        CloudController*                m_cloudController;
        ConfigurationManager&	        m_configurationManager;
		HMIController& 			        m_hmiController;

        ModbusTcpConfiguration_t        m_conf;
};
#endif
