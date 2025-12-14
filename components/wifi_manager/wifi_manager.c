/**
 * @file wifi_manager.c
 * @brief WiFi Manager Implementation
 */

#include "wifi_manager.h"
#include "captive_portal.h"
#include "dns_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "wifi_manager";

#define WIFI_MANAGER_NVS_NAMESPACE "wifi_config"
#define WIFI_MANAGER_NVS_SSID_KEY "ssid"
#define WIFI_MANAGER_NVS_PASSWORD_KEY "password"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// Global state
static wifi_manager_config_t g_config;
static wifi_manager_status_t g_status = WIFI_MANAGER_IDLE;
static wifi_manager_event_cb_t g_event_callback = NULL;
static void *g_event_user_data = NULL;

static esp_netif_t *sta_netif = NULL;
static esp_netif_t *ap_netif = NULL;
static EventGroupHandle_t wifi_event_group;

static int s_retry_num = 0;
static wifi_ap_record_t s_scan_results[20];
static uint16_t s_scan_count = 0;

/**
 * @brief Trigger event callback
 */
static void trigger_event(wifi_manager_event_t event)
{
    if (g_event_callback) {
        g_event_callback(event, g_event_user_data);
    }
}

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < g_config.max_retry_attempts) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to AP (%d/%d)", s_retry_num, g_config.max_retry_attempts);
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi");
        }
        g_status = WIFI_MANAGER_DISCONNECTED;
        trigger_event(WIFI_MANAGER_EVENT_STA_DISCONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        g_status = WIFI_MANAGER_CONNECTED;
        trigger_event(WIFI_MANAGER_EVENT_STA_CONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station "MACSTR" left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
        s_scan_count = 20;
        esp_wifi_scan_get_ap_records(&s_scan_count, s_scan_results);
        ESP_LOGI(TAG, "Scan done, found %d networks", s_scan_count);
        trigger_event(WIFI_MANAGER_EVENT_SCAN_DONE);
    }
}

wifi_manager_config_t wifi_manager_get_default_config(void)
{
    wifi_manager_config_t config = {
        .ap_ssid = "SmartLove-Setup",
        .ap_password = "",  // Open network
        .ap_channel = 1,
        .ap_max_connections = 4,
        .portal_timeout_s = 0,  // No timeout
        .max_retry_attempts = 5
    };
    return config;
}

esp_err_t wifi_manager_init(const wifi_manager_config_t *config)
{
    if (config) {
        g_config = *config;
    } else {
        g_config = wifi_manager_get_default_config();
    }

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default WiFi station and AP
    sta_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Create event group
    wifi_event_group = xEventGroupCreate();

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_LOGI(TAG, "WiFi Manager initialized");
    ESP_LOGI(TAG, "AP SSID: %s", g_config.ap_ssid);
    
    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    // Check if we have saved credentials
    if (wifi_manager_has_credentials()) {
        ESP_LOGI(TAG, "Found saved WiFi credentials, attempting to connect...");
        
        char ssid[33] = {0};
        char password[65] = {0};
        
        nvs_handle_t nvs_handle;
        if (nvs_open(WIFI_MANAGER_NVS_NAMESPACE, NVS_READONLY, &nvs_handle) == ESP_OK) {
            size_t ssid_len = sizeof(ssid);
            size_t password_len = sizeof(password);
            nvs_get_str(nvs_handle, WIFI_MANAGER_NVS_SSID_KEY, ssid, &ssid_len);
            nvs_get_str(nvs_handle, WIFI_MANAGER_NVS_PASSWORD_KEY, password, &password_len);
            nvs_close(nvs_handle);
            
            ESP_LOGI(TAG, "Connecting to: %s", ssid);
            return wifi_manager_connect(ssid, password, false);
        }
    }

    // No credentials, start AP mode
    ESP_LOGI(TAG, "No saved credentials, starting AP mode...");
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    
    wifi_config_t wifi_config = {
        .ap = {
            .channel = g_config.ap_channel,
            .max_connection = g_config.ap_max_connections,
            .authmode = (strlen(g_config.ap_password) == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK
        },
    };
    
    strlcpy((char *)wifi_config.ap.ssid, g_config.ap_ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(g_config.ap_ssid);
    
    if (strlen(g_config.ap_password) > 0) {
        strlcpy((char *)wifi_config.ap.password, g_config.ap_password, sizeof(wifi_config.ap.password));
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Start captive portal
    vTaskDelay(pdMS_TO_TICKS(1000));
    dns_server_start();
    captive_portal_start();
    
    g_status = WIFI_MANAGER_AP_MODE;
    trigger_event(WIFI_MANAGER_EVENT_AP_STARTED);
    
    ESP_LOGI(TAG, "AP mode started - SSID: %s", g_config.ap_ssid);
    ESP_LOGI(TAG, "Connect to configure WiFi at: http://192.168.4.1");
    
    return ESP_OK;
}

esp_err_t wifi_manager_stop(void)
{
    captive_portal_stop();
    dns_server_stop();
    esp_wifi_stop();
    
    g_status = WIFI_MANAGER_IDLE;
    
    ESP_LOGI(TAG, "WiFi Manager stopped");
    return ESP_OK;
}

esp_err_t wifi_manager_register_event_callback(wifi_manager_event_cb_t callback, void *user_data)
{
    g_event_callback = callback;
    g_event_user_data = user_data;
    return ESP_OK;
}

wifi_manager_status_t wifi_manager_get_status(void)
{
    return g_status;
}

bool wifi_manager_has_credentials(void)
{
    nvs_handle_t nvs_handle;
    if (nvs_open(WIFI_MANAGER_NVS_NAMESPACE, NVS_READONLY, &nvs_handle) != ESP_OK) {
        return false;
    }
    
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(nvs_handle, WIFI_MANAGER_NVS_SSID_KEY, NULL, &required_size);
    nvs_close(nvs_handle);
    
    return (err == ESP_OK && required_size > 1);
}

esp_err_t wifi_manager_save_credentials(const char *ssid, const char *password)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_MANAGER_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(nvs_handle, WIFI_MANAGER_NVS_SSID_KEY, ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs_handle, WIFI_MANAGER_NVS_PASSWORD_KEY, password ? password : "");
    }
    
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
    }
    
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials saved");
        trigger_event(WIFI_MANAGER_EVENT_CONFIG_SAVED);
    }
    
    return err;
}

esp_err_t wifi_manager_clear_credentials(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(WIFI_MANAGER_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    nvs_erase_key(nvs_handle, WIFI_MANAGER_NVS_SSID_KEY);
    nvs_erase_key(nvs_handle, WIFI_MANAGER_NVS_PASSWORD_KEY);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "WiFi credentials cleared");
    return ESP_OK;
}

esp_err_t wifi_manager_get_saved_ssid(char *ssid, size_t max_len)
{
    if (!ssid || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    if (nvs_open(WIFI_MANAGER_NVS_NAMESPACE, NVS_READONLY, &nvs_handle) != ESP_OK) {
        return ESP_FAIL;
    }

    size_t len = max_len;
    esp_err_t err = nvs_get_str(nvs_handle, WIFI_MANAGER_NVS_SSID_KEY, ssid, &len);
    nvs_close(nvs_handle);
    
    return err;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password, bool save_credentials)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }

    // Stop AP mode if running
    if (g_status == WIFI_MANAGER_AP_MODE) {
        captive_portal_stop();
        dns_server_stop();
        trigger_event(WIFI_MANAGER_EVENT_AP_STOPPED);
    }

    // Save credentials if requested
    if (save_credentials) {
        wifi_manager_save_credentials(ssid, password);
    }

    // Configure WiFi station
    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (password) {
        strlcpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_retry_num = 0;
    g_status = WIFI_MANAGER_CONNECTING;

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(30000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi: %s", ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi: %s", ssid);
        
        // Restart AP mode
        wifi_manager_start();
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "Connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t wifi_manager_scan(void)
{
    g_status = WIFI_MANAGER_SCANNING;
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    return esp_wifi_scan_start(&scan_config, false);
}

esp_err_t wifi_manager_get_scan_results(void *ap_records, uint16_t max_aps, uint16_t *num_aps)
{
    if (!ap_records || !num_aps) {
        return ESP_ERR_INVALID_ARG;
    }

    *num_aps = (s_scan_count < max_aps) ? s_scan_count : max_aps;
    memcpy(ap_records, s_scan_results, sizeof(wifi_ap_record_t) * (*num_aps));
    
    return ESP_OK;
}
