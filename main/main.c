/*
 * SmartLove - Main Application
 * 
 * Compatible with ESP-IDF 4.x and 5.x
 * 
 * This is a template for future development
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "smartlove_utils.h"

static const char *TAG = "SmartLove";

/**
 * @brief Print chip information
 * 
 * This function is compatible with both IDF 4.x and 5.x
 */
static void print_chip_info(void)
{
    esp_chip_info_t chip_info;
    
    esp_chip_info(&chip_info);
    
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "   SmartLove System Info");
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Chip: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Features: WiFi%s%s",
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "Free heap: %u bytes", (unsigned int)esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "=================================");
}

/**
 * @brief Main application entry point
 * 
 * This is the starting point of the SmartLove application.
 * It demonstrates basic ESP-IDF functionality and serves as
 * a foundation for future features.
 */
void app_main(void)
{
    // Initialize utilities
    ESP_ERROR_CHECK(smartlove_utils_init());
    
    // Print system information
    print_chip_info();
    
    ESP_LOGI(TAG, "Application started successfully!");
    ESP_LOGI(TAG, "IDF 5.x compatible: %s", 
             smartlove_is_idf5_or_higher() ? "Yes" : "No");
    ESP_LOGI(TAG, "Ready for development...");
    
    // Main application loop
    int counter = 0;
    uint64_t start_time = smartlove_get_uptime_ms();
    
    while (1) {
        uint64_t uptime = smartlove_get_uptime_ms() - start_time;
        
        ESP_LOGI(TAG, "Heartbeat: %d | Uptime: %llu ms | Free heap: %u bytes", 
                 counter++, uptime, (unsigned int)esp_get_free_heap_size());
        
        // Wait 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // TODO: Add your application logic here
        // - Sensor reading
        // - WiFi connection
        // - Data processing
        // - Communication protocols
    }
}
