/**
 * @file smartlove_mqtt.c
 * @brief MQTT Client Implementation (IDF 4.x compatible)
 */

#include "smartlove_mqtt.h"  // Our header
#include "mqtt_config.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"     // ESP-IDF MQTT component
#include "esp_mac.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "mqtt_client";

// MQTT client handle
static esp_mqtt_client_handle_t mqtt_client = NULL;

// Connection status
static mqtt_status_t current_status = MQTT_STATUS_DISCONNECTED;

// Chip ID and topics
static char chip_id[32] = {0};
static char client_id[64] = {0};
static char topic_out[128] = {0};
static char topic_in[128] = {0};
static char topic_status[128] = {0};

// Callbacks
static mqtt_message_callback_t message_callback = NULL;
static void *message_callback_user_data = NULL;
static mqtt_status_callback_t status_callback = NULL;
static void *status_callback_user_data = NULL;

/**
 * @brief Get the chip ID as a hex string
 */
static void get_chip_id_string(char *buffer, size_t size)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    snprintf(buffer, size, "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * @brief Build MQTT topics based on chip ID
 */
static void build_topics(void)
{
    get_chip_id_string(chip_id, sizeof(chip_id));
    
    // Build client ID: smartlove_<chipID>
    snprintf(client_id, sizeof(client_id), "%s%s",
             MQTT_CLIENT_ID_PREFIX, chip_id);
    
    // Build topics: SmartLove/<chipID>/out
    snprintf(topic_out, sizeof(topic_out), "%s/%s/%s",
             MQTT_TOPIC_PREFIX, chip_id, MQTT_TOPIC_OUT_SUFFIX);
    
    // SmartLove/<chipID>/in
    snprintf(topic_in, sizeof(topic_in), "%s/%s/%s",
             MQTT_TOPIC_PREFIX, chip_id, MQTT_TOPIC_IN_SUFFIX);
    
    // SmartLove/<chipID>/status
    snprintf(topic_status, sizeof(topic_status), "%s/%s/%s",
             MQTT_TOPIC_PREFIX, chip_id, MQTT_TOPIC_STATUS_SUFFIX);
    
    ESP_LOGI(TAG, "Chip ID: %s", chip_id);
    ESP_LOGI(TAG, "Client ID: %s", client_id);
    ESP_LOGI(TAG, "Topic OUT: %s", topic_out);
    ESP_LOGI(TAG, "Topic IN: %s", topic_in);
}

/**
 * @brief Update connection status and notify callback
 */
static void update_status(mqtt_status_t new_status)
{
    if (current_status != new_status) {
        current_status = new_status;
        ESP_LOGI(TAG, "Status changed to: %d", new_status);
        
        if (status_callback != NULL) {
            status_callback(new_status, status_callback_user_data);
        }
    }
}

/**
 * @brief MQTT event handler (IDF 4.x style)
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            update_status(MQTT_STATUS_CONNECTED);
            
            // Subscribe to incoming topic
            int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic_in, MQTT_QOS_LEVEL);
            ESP_LOGI(TAG, "Subscribed to %s, msg_id=%d", topic_in, msg_id);
            
            // Publish online status
            if (MQTT_LWT_ENABLED) {
                esp_mqtt_client_publish(mqtt_client, topic_status, "online",
                                      0, MQTT_LWT_QOS, MQTT_LWT_RETAIN);
            }
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            update_status(MQTT_STATUS_DISCONNECTED);
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGD(TAG, "Data: %.*s", event->data_len, event->data);
            
            // Call message callback
            if (message_callback != NULL) {
                message_callback(event->topic, event->topic_len,
                               event->data, event->data_len,
                               message_callback_user_data);
            }
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            update_status(MQTT_STATUS_ERROR);
            
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "Last errno: %d", event->error_handle->esp_transport_sock_errno);
            }
            break;
            
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            update_status(MQTT_STATUS_CONNECTING);
            break;
            
        default:
            ESP_LOGD(TAG, "Other event id: %d", event->event_id);
            break;
    }
    
    return ESP_OK;
}

esp_err_t mqtt_client_init(void)
{
    if (mqtt_client != NULL) {
        ESP_LOGW(TAG, "MQTT client already initialized");
        return ESP_OK;
    }
    
    // Build topics with chip ID
    build_topics();
    
    // Configure MQTT client (IDF 4.x style)
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URI,
        .client_id = client_id,
        .keepalive = MQTT_KEEPALIVE_SECONDS,
        .reconnect_timeout_ms = MQTT_RECONNECT_TIMEOUT_MS,
        .disable_auto_reconnect = false,
        .buffer_size = MQTT_BUFFER_SIZE,
        .task_stack = MQTT_TASK_STACK_SIZE,
        .task_prio = MQTT_TASK_PRIORITY,
        .event_handle = mqtt_event_handler,
    };
    
    // Set username/password if provided
    if (strlen(MQTT_USERNAME) > 0) {
        mqtt_cfg.username = MQTT_USERNAME;
    }
    if (strlen(MQTT_PASSWORD) > 0) {
        mqtt_cfg.password = MQTT_PASSWORD;
    }
    
    // Configure Last Will and Testament
    if (MQTT_LWT_ENABLED) {
        mqtt_cfg.lwt_topic = topic_status;
        mqtt_cfg.lwt_msg = MQTT_LWT_MESSAGE;
        mqtt_cfg.lwt_msg_len = strlen(MQTT_LWT_MESSAGE);
        mqtt_cfg.lwt_qos = MQTT_LWT_QOS;
        mqtt_cfg.lwt_retain = MQTT_LWT_RETAIN;
    }
    
    // Create MQTT client
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "MQTT client initialized");
    return ESP_OK;
}

esp_err_t mqtt_client_start(void)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    update_status(MQTT_STATUS_CONNECTING);
    
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client");
        update_status(MQTT_STATUS_ERROR);
        return err;
    }
    
    ESP_LOGI(TAG, "MQTT client started");
    return ESP_OK;
}

esp_err_t mqtt_client_stop(void)
{
    if (mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT client not initialized");
        return ESP_OK;
    }
    
    // Publish offline status before disconnecting
    if (MQTT_LWT_ENABLED && current_status == MQTT_STATUS_CONNECTED) {
        esp_mqtt_client_publish(mqtt_client, topic_status, "offline",
                              0, MQTT_LWT_QOS, MQTT_LWT_RETAIN);
    }
    
    esp_err_t err = esp_mqtt_client_stop(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop MQTT client");
        return err;
    }
    
    update_status(MQTT_STATUS_DISCONNECTED);
    ESP_LOGI(TAG, "MQTT client stopped");
    return ESP_OK;
}

bool mqtt_client_is_connected(void)
{
    return current_status == MQTT_STATUS_CONNECTED;
}

mqtt_status_t mqtt_client_get_status(void)
{
    return current_status;
}

esp_err_t mqtt_client_register_message_callback(mqtt_message_callback_t callback,
                                               void *user_data)
{
    message_callback = callback;
    message_callback_user_data = user_data;
    ESP_LOGI(TAG, "Message callback registered");
    return ESP_OK;
}

esp_err_t mqtt_client_register_status_callback(mqtt_status_callback_t callback,
                                              void *user_data)
{
    status_callback = callback;
    status_callback_user_data = user_data;
    ESP_LOGI(TAG, "Status callback registered");
    return ESP_OK;
}

esp_err_t mqtt_client_send(const char *message)
{
    if (message == NULL) {
        ESP_LOGE(TAG, "Message is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    return mqtt_client_send_data(message, strlen(message));
}

esp_err_t mqtt_client_send_data(const char *data, int len)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Invalid data or length");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (current_status != MQTT_STATUS_CONNECTED) {
        ESP_LOGW(TAG, "MQTT not connected, cannot send");
        return ESP_ERR_INVALID_STATE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic_out, data, len,
                                        MQTT_QOS_LEVEL, MQTT_RETAIN_FLAG);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish message");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Message sent to %s, msg_id=%d", topic_out, msg_id);
    return ESP_OK;
}

esp_err_t mqtt_client_publish(const char *topic, const char *message)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (topic == NULL || message == NULL) {
        ESP_LOGE(TAG, "Topic or message is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (current_status != MQTT_STATUS_CONNECTED) {
        ESP_LOGW(TAG, "MQTT not connected, cannot publish");
        return ESP_ERR_INVALID_STATE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, message,
                                        strlen(message), MQTT_QOS_LEVEL,
                                        MQTT_RETAIN_FLAG);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish message to %s", topic);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Message published to %s, msg_id=%d", topic, msg_id);
    return ESP_OK;
}

esp_err_t mqtt_client_get_chip_id(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(chip_id) == 0) {
        get_chip_id_string(chip_id, sizeof(chip_id));
    }
    
    snprintf(buffer, buffer_size, "%s", chip_id);
    return ESP_OK;
}

esp_err_t mqtt_client_get_out_topic(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    snprintf(buffer, buffer_size, "%s", topic_out);
    return ESP_OK;
}

esp_err_t mqtt_client_get_in_topic(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    snprintf(buffer, buffer_size, "%s", topic_in);
    return ESP_OK;
}

esp_err_t mqtt_client_deinit(void)
{
    if (mqtt_client == NULL) {
        return ESP_OK;
    }
    
    // Stop client if running
    if (current_status != MQTT_STATUS_DISCONNECTED) {
        mqtt_client_stop();
    }
    
    esp_err_t err = esp_mqtt_client_destroy(mqtt_client);
    mqtt_client = NULL;
    
    // Clear callbacks
    message_callback = NULL;
    message_callback_user_data = NULL;
    status_callback = NULL;
    status_callback_user_data = NULL;
    
    current_status = MQTT_STATUS_DISCONNECTED;
    
    ESP_LOGI(TAG, "MQTT client deinitialized");
    return err;
}
