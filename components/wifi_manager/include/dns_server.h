/**
 * @file dns_server.h
 * @brief Simple DNS Server for Captive Portal
 * 
 * Redirects all DNS queries to the AP IP address
 * to enable captive portal functionality
 */

#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start DNS server
 * 
 * @return ESP_OK on success
 */
esp_err_t dns_server_start(void);

/**
 * @brief Stop DNS server
 * 
 * @return ESP_OK on success
 */
esp_err_t dns_server_stop(void);

/**
 * @brief Check if DNS server is running
 * 
 * @return true if running
 */
bool dns_server_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // DNS_SERVER_H
