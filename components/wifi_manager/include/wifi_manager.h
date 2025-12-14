/**
 * @file wifi_manager.h
 * @brief WiFi Manager with Captive Portal for ESP32
 * 
 * Features:
 * - Auto-connect to saved WiFi
 * - Fallback to AP mode on connection failure
 * - Captive portal for WiFi configuration
 * - NVS storage for credentials
 * - IDF 4.x and 5.x compatible
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/** WiFi Manager Configuration */
typedef struct {
    char ap_ssid[32];           /**< AP SSID when in config mode */
    char ap_password[64];       /**< AP password (empty = open) */
    uint8_t ap_channel;         /**< AP channel (1-13) */
    uint8_t ap_max_connections; /**< Max AP connections */
    uint16_t portal_timeout_s;  /**< Portal timeout in seconds (0 = no timeout) */
    uint8_t max_retry_attempts; /**< Max WiFi connection attempts */
} wifi_manager_config_t;

/** WiFi Manager Status */
typedef enum {
    WIFI_MANAGER_IDLE = 0,
    WIFI_MANAGER_SCANNING,
    WIFI_MANAGER_CONNECTING,
    WIFI_MANAGER_CONNECTED,
    WIFI_MANAGER_DISCONNECTED,
    WIFI_MANAGER_AP_MODE,
    WIFI_MANAGER_ERROR
} wifi_manager_status_t;

/** WiFi Manager Events */
typedef enum {
    WIFI_MANAGER_EVENT_STA_CONNECTED = 0,
    WIFI_MANAGER_EVENT_STA_DISCONNECTED,
    WIFI_MANAGER_EVENT_AP_STARTED,
    WIFI_MANAGER_EVENT_AP_STOPPED,
    WIFI_MANAGER_EVENT_SCAN_DONE,
    WIFI_MANAGER_EVENT_CONFIG_SAVED
} wifi_manager_event_t;

/** WiFi Manager Event Callback */
typedef void (*wifi_manager_event_cb_t)(wifi_manager_event_t event, void *user_data);

/**
 * @brief Get default WiFi Manager configuration
 * 
 * @return Default configuration
 */
wifi_manager_config_t wifi_manager_get_default_config(void);

/**
 * @brief Initialize WiFi Manager
 * 
 * @param config Configuration (NULL = use defaults)
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(const wifi_manager_config_t *config);

/**
 * @brief Start WiFi Manager
 * 
 * Attempts to connect to saved WiFi credentials.
 * If connection fails, starts AP mode with captive portal.
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start(void);

/**
 * @brief Stop WiFi Manager
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_stop(void);

/**
 * @brief Register event callback
 * 
 * @param callback Event callback function
 * @param user_data User data passed to callback
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_register_event_callback(wifi_manager_event_cb_t callback, void *user_data);

/**
 * @brief Get current WiFi Manager status
 * 
 * @return Current status
 */
wifi_manager_status_t wifi_manager_get_status(void);

/**
 * @brief Check if WiFi credentials are saved
 * 
 * @return true if credentials exist in NVS
 */
bool wifi_manager_has_credentials(void);

/**
 * @brief Save WiFi credentials to NVS
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_save_credentials(const char *ssid, const char *password);

/**
 * @brief Clear saved WiFi credentials
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_clear_credentials(void);

/**
 * @brief Get saved WiFi SSID
 * 
 * @param ssid Buffer to store SSID
 * @param max_len Buffer size
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_saved_ssid(char *ssid, size_t max_len);

/**
 * @brief Manually connect to WiFi
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @param save_credentials Save to NVS if true
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password, bool save_credentials);

/**
 * @brief Start WiFi scan
 * 
 * Results will be available via WIFI_MANAGER_EVENT_SCAN_DONE event
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_scan(void);

/**
 * @brief Get scan results
 * 
 * @param ap_records Buffer to store AP records
 * @param max_aps Maximum number of APs to retrieve
 * @param num_aps Actual number of APs found
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_scan_results(void *ap_records, uint16_t max_aps, uint16_t *num_aps);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
