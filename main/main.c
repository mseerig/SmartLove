/*
 * SmartLove - Main Application
 * 
 * Compatible with ESP-IDF 4.x and 5.x
 * 
 * Features:
 * - WiFi Manager with Captive Portal
 * - Auto-connect to saved WiFi
 * - Fallback to AP mode for configuration
 * - MQTT client with chip-ID based topics
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
#include "smartlove_mqtt.h"

static const char *TAG = "SmartLove";

// MQTT connection flag
static bool mqtt_started = false;

/**
 * @brief MQTT message callback
 * 
 * Called when a message is received on SmartLove/<chipID>/in
 */
static void mqtt_message_handler(const char *topic, int topic_len,
                                 const char *data, int data_len,
                                 void *user_data)
{
    ESP_LOGI(TAG, "📨 MQTT Message received:");
    ESP_LOGI(TAG, "   Topic: %.*s", topic_len, topic);
    ESP_LOGI(TAG, "   Data: %.*s", data_len, data);
    
    // Example: Simple command parsing
    if (data_len > 0 && data_len < 256) {
        char message[256];
        memcpy(message, data, data_len);
        message[data_len] = '\0';
        
        ESP_LOGI(TAG, "💬 Processing command: %s", message);
        
        if (strcmp(message, "PING") == 0) {
            mqtt_client_send("PONG");
        } else if (strcmp(message, "STATUS") == 0) {
            char status_msg[128];
            snprintf(status_msg, sizeof(status_msg), 
                    "ONLINE|Heap:%u|Uptime:%llu", 
                    (unsigned int)esp_get_free_heap_size(),
                    smartlove_get_uptime_ms() / 1000);
            mqtt_client_send(status_msg);
        }
    }
}

/**
 * @brief MQTT status callback
 */
static void mqtt_status_handler(mqtt_status_t status, void *user_data)
{
    switch (status) {
        case MQTT_STATUS_CONNECTED:
            ESP_LOGI(TAG, "🔗 MQTT Connected");
            
            char out_topic[128], in_topic[128], chip_id[32];
            mqtt_client_get_chip_id(chip_id, sizeof(chip_id));
            mqtt_client_get_out_topic(out_topic, sizeof(out_topic));
            mqtt_client_get_in_topic(in_topic, sizeof(in_topic));
            
            ESP_LOGI(TAG, "   Chip ID: %s", chip_id);
            ESP_LOGI(TAG, "   Publish to: %s", out_topic);
            ESP_LOGI(TAG, "   Subscribe: %s", in_topic);
            
            mqtt_client_send("SmartLove device online!");
            break;
            
        case MQTT_STATUS_DISCONNECTED:
            ESP_LOGW(TAG, "🔌 MQTT Disconnected");
            break;
            
        case MQTT_STATUS_CONNECTING:
            ESP_LOGI(TAG, "🔄 MQTT Connecting...");
            break;
            
        case MQTT_STATUS_ERROR:
            ESP_LOGE(TAG, "❌ MQTT Connection Error");
            break;
    }
}

/**
 * @brief WiFi Manager event callback
 */
static void wifi_event_callback(wifi_manager_event_t event, void *user_data)
{
    switch (event) {
        case WIFI_MANAGER_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "📶 WiFi Connected!");
            
            if (!mqtt_started) {
                ESP_LOGI(TAG, "Starting MQTT client...");
                if (mqtt_client_start() == ESP_OK) {
                    mqtt_started = true;
                } else {
                    ESP_LOGE(TAG, "Failed to start MQTT client");
                }
            }
            break;
            
        case WIFI_MANAGER_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "📵 WiFi Disconnected");
            
            if (mqtt_started) {
                ESP_LOGI(TAG, "Stopping MQTT client...");
                mqtt_client_stop();
                mqtt_started = false;
            }
            break;
            
        case WIFI_MANAGER_EVENT_AP_STARTED:
            ESP_LOGI(TAG, "🔧 Configuration Portal Started");
            ESP_LOGI(TAG, "   Connect to WiFi AP to configure");
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
 * @brief Main application entry point
 */
void app_main(void)
{
    // Initialize utilities
    ESP_ERROR_CHECK(smartlove_utils_init());
    
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "   SmartLove Application");
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "IDF 5.x compatible: %s", 
             smartlove_is_idf5_or_higher() ? "Yes" : "No");
    
    // Initialize WiFi Manager
    ESP_LOGI(TAG, "Initializing WiFi Manager...");
    wifi_manager_config_t config = wifi_manager_get_default_config();
    
    snprintf(config.ap_ssid, sizeof(config.ap_ssid), "SmartLove-%02X%02X", 
             0xFF & (esp_random() >> 8), 0xFF & esp_random());
    
    ESP_ERROR_CHECK(wifi_manager_init(&config));
    ESP_ERROR_CHECK(wifi_manager_register_event_callback(wifi_event_callback, NULL));
    ESP_ERROR_CHECK(wifi_manager_start());
    
    // Initialize MQTT client
    ESP_LOGI(TAG, "Initializing MQTT client...");
    ESP_ERROR_CHECK(mqtt_client_init());
    ESP_ERROR_CHECK(mqtt_client_register_message_callback(mqtt_message_handler, NULL));
    ESP_ERROR_CHECK(mqtt_client_register_status_callback(mqtt_status_handler, NULL));
    ESP_LOGI(TAG, "MQTT client initialized (will start when WiFi connects)");
    
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
        ESP_LOGI(TAG, "📱 Connect to '%s' to configure", config.ap_ssid);
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
            case WIFI_MANAGER_IDLE: 
                status_str = "Idle"; 
                break;
            case WIFI_MANAGER_SCANNING: 
                status_str = "Scanning"; 
                break;
            case WIFI_MANAGER_CONNECTING: 
                status_str = "Connecting"; 
                break;
            case WIFI_MANAGER_CONNECTED: 
                status_str = "Connected ✓"; 
                break;
            case WIFI_MANAGER_DISCONNECTED: 
                status_str = "Disconnected"; 
                break;
            case WIFI_MANAGER_AP_MODE: 
                status_str = "AP Mode (Config)"; 
                break;
            case WIFI_MANAGER_ERROR: 
                status_str = "Error"; 
                break;
        }
        
        ESP_LOGI(TAG, "⏱ Uptime: %llu s | WiFi: %s | Heap: %u bytes", 
                 uptime / 1000, status_str, (unsigned int)esp_get_free_heap_size());
        
        // Send MQTT heartbeat if connected
        if (mqtt_client_is_connected() && counter % 6 == 0) { // Every 60 seconds
            char heartbeat[128];
            snprintf(heartbeat, sizeof(heartbeat),
                    "Heartbeat|Uptime:%llu|Heap:%u|Counter:%d",
                    uptime / 1000,
                    (unsigned int)esp_get_free_heap_size(),
                    counter);
            mqtt_client_send(heartbeat);
            ESP_LOGI(TAG, "📤 MQTT Heartbeat sent");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));
        counter++;
    }
}
