/**
 * @file smartlove_utils.h
 * @brief SmartLove Utility Functions
 * 
 * This component provides utility functions for the SmartLove project.
 * Compatible with ESP-IDF 4.x and 5.x
 */

#ifndef SMARTLOVE_UTILS_H
#define SMARTLOVE_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_idf_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SmartLove utilities
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t smartlove_utils_init(void);

/**
 * @brief Get system uptime in milliseconds
 * 
 * @return System uptime in milliseconds
 */
uint64_t smartlove_get_uptime_ms(void);

/**
 * @brief Format a MAC address to string
 * 
 * @param mac MAC address bytes (6 bytes)
 * @param out Output buffer (minimum 18 bytes)
 * @return ESP_OK on success
 */
esp_err_t smartlove_format_mac(const uint8_t *mac, char *out);

/**
 * @brief Check if running on IDF 5.x
 * 
 * @return true if IDF 5.x or higher, false otherwise
 */
bool smartlove_is_idf5_or_higher(void);

#ifdef __cplusplus
}
#endif

#endif // SMARTLOVE_UTILS_H
