/*
 * SmartLove - Main Application
 * 
 * Compatible with ESP-IDF 4.x and 5.x
 * 
 * Features:
 * - WiFi Manager with Captive Portal
 * - Auto-connect to saved WiFi
 * - Fallback to AP mode for configuration
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "smartlove_utils.h"
#include "wifi_manager.h"

static const char *TAG = "SmartLove";

/**
 * @brief WiFi Manager event callback
 */
static void wifi_event_callback(wifi_manager_event_t event, void *user_data)
{
    switch (event) {
        case WIFI_MANAGER_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "📶 WiFi Connected!");
            break;
        case WIFI_MANAGER_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "📵 WiFi Disconnected");
            break;
        case WIFI_MANAGER_EVENT_AP_STARTED:
            ESP_LOGI(TAG, "🔧 Configuration Portal Started");
            ESP_LOGI(TAG, "   Connect to WiFi: 'SmartLove-Setup'");
            ESP_LOGI(TAG, "   Open browser at: http://192.168.4.1");
            break;
        case WIFI_MANAGER_EVENT_AP_STOPPED:
            ESP_LOGI(TAG, "🔧 Configuration Portal Stopped");
            break;
        case WIFI_MANAGER_EVENT_SCAN_DONE:
            ESP_LOGI(TAG, "🔍 WiFi Scan Complete");
            break;
        case WIFI_MANAGER_EVENT_CONFIG_SAVED:
            ESP_LOGI(TAG, "💾 WiFi Configuration Saved");
            break;
        default:
            break;
    }
}

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
    
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "   SmartLove Application");
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "IDF 5.x compatible: %s", 
             smartlove_is_idf5_or_higher() ? "Yes" : "No");
    
    // Initialize WiFi Manager
    ESP_LOGI(TAG, "Initializing WiFi Manager...");
    wifi_manager_config_t wifi_config = wifi_manager_get_default_config();
    
    // Customize AP name if needed
    snprintf(wifi_config.ap_ssid, sizeof(wifi_config.ap_ssid), "SmartLove-%02X%02X", 
             0xFF & (esp_random() >> 8), 0xFF & esp_random());
    
    ESP_ERROR_CHECK(wifi_manager_init(&wifi_config));
    ESP_ERROR_CHECK(wifi_manager_register_event_callback(wifi_event_callback, NULL));
    ESP_ERROR_CHECK(wifi_manager_start());
    
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Application started successfully!");
    
    // Check if we have WiFi credentials
    if (wifi_manager_has_credentials()) {
        char saved_ssid[33] = {0};
        if (wifi_manager_get_saved_ssid(saved_ssid, sizeof(saved_ssid)) == ESP_OK) {
            ESP_LOGI(TAG, "Saved WiFi: %s", saved_ssid);
        }
    } else {
        ESP_LOGI(TAG, "No WiFi configured yet");
        ESP_LOGI(TAG, "📱 Connect to '%s' to configure", wifi_config.ap_ssid);
    }
    
    ESP_LOGI(TAG, "=================================");
    
    // Main application loop
    int counter = 0;
    uint64_t start_time = smartlove_get_uptime_ms();
    
    while (1) {
        uint64_t uptime = smartlove_get_uptime_ms() - start_time;
        wifi_manager_status_t wifi_status = wifi_manager_get_status();
        
        const char *status_str = "Unknown";
        switch (wifi_status) {
            case WIFI_MANAGER_IDLE: status_str = "Idle"; break;
            case WIFI_MANAGER_SCANNING: status_str = "Scanning"; break;
            case WIFI_MANAGER_CONNECTING: status_str = "Connecting"; break;
            case WIFI_MANAGER_CONNECTED: status_str = "Connected ✓"; break;
            case WIFI_MANAGER_DISCONNECTED: status_str = "Disconnected"; break;
            case WIFI_MANAGER_AP_MODE: status_str = "AP Mode (Config)"; break;
            case WIFI_MANAGER_ERROR: status_str = "Error"; break;
        }
        
        ESP_LOGI(TAG, "⏱ Uptime: %llu s | WiFi: %s | Heap: %u bytes", 
                 uptime / 1000, status_str, (unsigned int)esp_get_free_heap_size());
        
        // Wait 10 seconds
        vTaskDelay(pdMS_TO_TICKS(10000));
        counter++;
        
        // TODO: Add your application logic here
        // - Sensor reading
        // - Cloud communication
        // - Data processing
        // - MQTT/HTTP requests
    }
}
