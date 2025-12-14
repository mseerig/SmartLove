/**
 * @file captive_portal.h
 * @brief Captive Portal HTTP Server for WiFi Configuration
 */

#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start captive portal HTTP server
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_start(void);

/**
 * @brief Stop captive portal HTTP server
 * 
 * @return ESP_OK on success
 */
esp_err_t captive_portal_stop(void);

/**
 * @brief Check if captive portal is running
 * 
 * @return true if running
 */
bool captive_portal_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // CAPTIVE_PORTAL_H
