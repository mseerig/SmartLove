/**
 * @file smartlove_utils.c
 * @brief SmartLove Utility Functions Implementation
 */

#include "smartlove_utils.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "smartlove_utils";

esp_err_t smartlove_utils_init(void)
{
    ESP_LOGI(TAG, "Initializing SmartLove Utilities");
    ESP_LOGI(TAG, "IDF Version: %s", esp_get_idf_version());
    
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    ESP_LOGI(TAG, "Running on ESP-IDF 5.x or higher");
#else
    ESP_LOGI(TAG, "Running on ESP-IDF 4.x");
#endif
    
    return ESP_OK;
}

uint64_t smartlove_get_uptime_ms(void)
{
    return esp_timer_get_time() / 1000ULL;
}

esp_err_t smartlove_format_mac(const uint8_t *mac, char *out)
{
    if (mac == NULL || out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    snprintf(out, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return ESP_OK;
}

bool smartlove_is_idf5_or_higher(void)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    return true;
#else
    return false;
#endif
}
