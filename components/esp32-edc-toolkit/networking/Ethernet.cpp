/*
 * Ethernet.cpp
 *
 *  Created on: 07.07.2017
 *      Author: marcel.seerig
 */

#include "Ethernet.hpp"
#include "FreeRTOS.hpp"
#include "GeneralUtils.hpp"

#include <lwip/sockets.h>
#include <stdbool.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "Definitions.hpp"


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "driver/spi_master.h"


#define ETH_SPI_HOST 			SPI2_HOST
#define ETH_SPI_CLOCK_MHZ 		25
#define ETH_SPI_PHY_ADDR		ESP_ETH_PHY_ADDR_AUTO

#define ETH_SPI_SCLK_GPIO 		36
#define ETH_SPI_MISO_GPIO 		37
#define ETH_SPI_MOSI_GPIO 		35
#define ETH_SPI_INT_GPIO		38
#define ETH_SPI_PHY_RESET_GPIO 	33
#define ETH_SPI_CS_GPIO		 	GPIO_NUM_34

uint8_t mac[6] = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};

static char LOGTAG[] = "Ethernet";
//ethernet_config_t Ethernet::config;

Ethernet *Ethernet::s_Instance = nullptr;

/**
 * @brief Constructor of the Ethernet interface
 * @param [in] configuration of the Ethernet interface
 */
Ethernet::Ethernet(){
	s_Instance = this;
	ESP_LOGD(LOGTAG, "Starting");

	if (esp_efuse_mac_get_custom(mac) != ESP_OK ) {
        esp_read_mac(mac, ESP_MAC_ETH);
        ESP_LOGD(LOGTAG, "No MAC burned, using Espressif MAC");
    }
    std::string macString = GeneralUtils::macToString(mac);
    ESP_LOGD(LOGTAG, "MAC: %s", macString.c_str());

    // Create instance(s) of esp-netif for SPI Ethernet(s)
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config_t cfg_spi = {
        .base = &esp_netif_config,
    	.driver = NULL,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    esp_netif_t *eth_netif_spi = NULL;

    esp_netif_config.if_key = "ETH_SPI_0";
    esp_netif_config.if_desc = "eth0";
    esp_netif_config.route_prio = 30;
    eth_netif_spi = esp_netif_new(&cfg_spi);


    // Init MAC and PHY configs to default
    eth_mac_config_t mac_config_spi = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config_spi = ETH_PHY_DEFAULT_CONFIG();

    // Install GPIO ISR handler to be able to service SPI Eth modlues interrupts
    gpio_install_isr_service(0);

    // Init SPI bus
    spi_device_handle_t spi_handle = NULL;
    spi_bus_config_t buscfg = {
		.mosi_io_num = ETH_SPI_MOSI_GPIO,
        .miso_io_num = ETH_SPI_MISO_GPIO,
        .sclk_io_num = ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,     ///< GPIO pin for spi data4 signal in octal mode, or -1 if not used.
        .data5_io_num = -1,     ///< GPIO pin for spi data5 signal in octal mode, or -1 if not used.
        .data6_io_num = -1,     ///< GPIO pin for spi data6 signal in octal mode, or -1 if not used.
        .data7_io_num = -1,
        .max_transfer_sz = 0,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Configure SPI interface and Ethernet driver for specific SPI module
    esp_eth_mac_t *mac_spi;
    esp_eth_phy_t *phy_spi;

	spi_device_interface_config_t devcfg = {
        .command_bits = 16,
        .address_bits = 8,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = ETH_SPI_CS_GPIO,
        .flags = 0,
        .queue_size = 20,
        .pre_cb= NULL,
        .post_cb= NULL,
    };

	ESP_ERROR_CHECK(spi_bus_add_device(ETH_SPI_HOST, &devcfg, &spi_handle));
	// w5500 ethernet driver is based on spi driver
	eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);

	// Set remaining GPIO numbers and configuration used by the SPI module
	w5500_config.int_gpio_num = ETH_SPI_INT_GPIO;	
	phy_config_spi.phy_addr = ETH_SPI_PHY_ADDR;
	phy_config_spi.reset_gpio_num = ETH_SPI_PHY_RESET_GPIO;

	mac_spi = esp_eth_mac_new_w5500(&w5500_config, &mac_config_spi);
	phy_spi = esp_eth_phy_new_w5500(&phy_config_spi);




	esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac_spi, phy_spi);
	ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config_spi, &m_hdl));

	/* The SPI Ethernet module might not have a burned factory MAC address, we cat to set it manually.
	02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
	*/
	
	ESP_ERROR_CHECK(esp_eth_ioctl(m_hdl, ETH_CMD_S_MAC_ADDR, mac));


	// attach Ethernet driver to TCP/IP stack
	ESP_ERROR_CHECK(esp_netif_attach(eth_netif_spi, esp_eth_new_netif_glue(m_hdl)));

}

/**
 * @brief Destructor of the Ethernet interface
 */
Ethernet::~Ethernet()
{
	s_Instance = nullptr;
	esp_eth_stop(m_hdl);
	esp_eth_driver_uninstall(m_hdl);
}

esp_err_t Ethernet::stop(void){
	return esp_eth_stop(m_hdl);
}

esp_err_t Ethernet::start(void){
	return esp_eth_start(m_hdl);
}

/**
 * @brief Get the MAC address of the Ethernet interface.
 * @return The MAC address of the Ethernet interface.
 */
/*
std::string Ethernet::getMac()
{
	uint8_t mac[6];
	esp_eth_get_mac(mac);
	auto mac_str = (char *)malloc(18);
	sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return std::string(std::move(mac_str));
} // getMac
*/

/**
 * @brief Get the Ethernet IP Info.
 * @return The Ethernet IP Info.
 */
tcpip_adapter_ip_info_t Ethernet::getIpInfo()
{
	tcpip_adapter_ip_info_t ipInfo;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo);
	return ipInfo;
} // getIpInfo

/**
 * @brief Get the current ESP32 IP form Ethernet.
 * @return The ESP32 IP.
 */
std::string Ethernet::getIp()
{
	tcpip_adapter_ip_info_t ipInfo = getIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.ip.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getIp

/**
 * @brief Get the current Ethernet netmask.
 * @return The Netmask IP.
 */
std::string Ethernet::getNetmask()
{
	tcpip_adapter_ip_info_t ipInfo = getIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.netmask.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getNetmask

/**
 * @brief Get the current Ethernet Gateway IP.
 * @return The Gateway IP.
 */
std::string Ethernet::getGateway()
{
	tcpip_adapter_ip_info_t ipInfo = getIpInfo();
	char ipAddrStr[30];
	inet_ntop(AF_INET, &ipInfo.gw.addr, ipAddrStr, sizeof(ipAddrStr));
	return std::string(ipAddrStr);
} // getGateway

/**
 * @brief Set the IP Info based on the IP address, gateway, and netmask.
 * @param [in] ip The IP address of our ESP32 as a string (e.g., "192.168.1.100").
 * @param [in] gw The gateway we should use as a string (e.g., "192.168.1.1").
 * @param [in] netmask Our TCP/IP netmask value as a string (e.g., "255.255.255.0").
 */
void Ethernet::setStaticIP(std::string ip, std::string gw, std::string netmask) {

    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("ETH_SPI_0");
    if (netif == NULL) {
        ESP_LOGE("LOGTAG", "Error getting the network interface handle.");
        return;
    }

	// Beenden Sie den DHCP-Client für diese Netzwerkschnittstelle
    esp_err_t err = esp_netif_dhcpc_stop(netif);
    if (err != ESP_OK) {
        ESP_LOGE("LOGTAG", "Error stopping DHCP client. Error code: %d (%s)", err, esp_err_to_name(err));
    }
    m_DHCPenabled = false;

	// setze statische ip
    esp_netif_ip_info_t ipInfo;
    inet_pton(AF_INET, ip.c_str(), &ipInfo.ip);
    inet_pton(AF_INET, gw.c_str(), &ipInfo.gw);
    inet_pton(AF_INET, netmask.c_str(), &ipInfo.netmask);

    err = esp_netif_set_ip_info(netif, &ipInfo);
    if (err != ESP_OK) {
        ESP_LOGE("LOGTAG", "Error setting static IP info. Error code: %d (%s)", err, esp_err_to_name(err));
        return;
    }

    ESP_LOGI("LOGTAG", "Static IP address set: %s", ip.c_str());
}

/**
 * @brief Enable dynamic IP configuration using DHCP for the Ethernet interface.
 */
void Ethernet::setDynamicIP(void) {
    // Start the DHCP client for the Ethernet interface
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("ETH_SPI_0");
    if (netif == NULL) {
        ESP_LOGE("LOGTAG", "Error getting the network interface handle.");
        return;
    }

    esp_err_t err = esp_netif_dhcpc_start(netif);
    if (err != ESP_OK) {
        ESP_LOGE("LOGTAG", "Error starting DHCP client. Error code: %d (%s)", err, esp_err_to_name(err));
        return;
    }

    // Set the DHCP enabled flag to true
    m_DHCPenabled = true;

    ESP_LOGI("LOGTAG", "Dynamic IP configuration enabled (DHCP).");
}

/**
 * @brief Retrieves the DNS information for the network interface.
 *
 * This function retrieves the IP address of the DNS server associated with the specified
 * network interface and DNS type.
 *
 * @param type The DNS type, e.g., ESP_NETIF_DNS_MAIN.
 *
 * @return The IP address of the DNS server as a string.
 *         If an error occurs, an appropriate error message is returned.
 *
 * @note This function requires the prior initialization of the network interface.
 *       Ensure that the required header files are included and the ESP-IDF library
 *       is correctly installed.
 */
std::string Ethernet::getDnsInfo(esp_netif_dns_type_t type){

	esp_netif_t* netif = esp_netif_get_handle_from_ifkey("ETH_SPI_0");
	if (netif == NULL) {
        ESP_LOGE("LOGTAG", "Fehler beim Abrufen der Netzwerkschnittstelle.");
        return "Error";
    }

	esp_netif_dns_info_t dns;
	esp_err_t err = esp_netif_get_dns_info(netif, type, &dns);
	if(err != ESP_OK) {
		ESP_LOGE("LOGTAG", "Fehler beim Lesen des DNS-Servers. Fehlercode: %d", err);
		return "Error";
	}

	// Convert to std::string
	std::string ipAddress(inet_ntoa(dns.ip.u_addr.ip4));
	return ipAddress;
}


/**
 * @brief Sets the DNS information for the network interface.
 *
 * This function sets the DNS server for the specified network interface
 * based on the given type and IP address.
 *
 * @param type The DNS type, e.g., ESP_NETIF_DNS_MAIN.
 * @param ip The IP address of the DNS server as a string.
 *
 * @return void
 *
 * @note This function requires prior initialization of the network interface.
 *       Ensure that the required header files are included and the ESP-IDF
 *       library is correctly installed.
 */
void Ethernet::setDnsInfo(esp_netif_dns_type_t type, std::string ip) {
    // Erstelle eine Instanz der Netzwerkschnittstelle
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("ETH_SPI_0");
    if (netif == NULL) {
        ESP_LOGE("LOGTAG", "Fehler beim Abrufen der Netzwerkschnittstelle.");
        return;
    }

    // Konvertiere die übergebene IP-Adresse in ip4_addr_t
    ip4_addr_t dnsServer;
    inet_pton(AF_INET, ip.c_str(), &(dnsServer));

    esp_netif_dns_info_t dnsInfo;
    dnsInfo.ip.type = IPADDR_TYPE_V4;
    ip4_addr_copy(dnsInfo.ip.u_addr.ip4, dnsServer);

    // Setze den DNS-Server für die Netzwerkschnittstelle
    esp_err_t err = esp_netif_set_dns_info(netif, type, &dnsInfo);
    if (err != ESP_OK) {
        ESP_LOGE("LOGTAG", "Fehler beim Setzen des DNS-Servers. Fehlercode: %d", err);
        return;
    }
}