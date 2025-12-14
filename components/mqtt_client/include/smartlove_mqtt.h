/**
 * @file smartlove_mqtt.h
 * @brief MQTT Client for SmartLove
 * 
 * Provides MQTT publish/subscribe functionality with automatic
 * topic management based on chip ID.
 */

#ifndef SMARTLOVE_MQTT_CLIENT_H
#define SMARTLOVE_MQTT_CLIENT_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MQTT connection status
 */
typedef enum {
    MQTT_STATUS_DISCONNECTED = 0,  ///< Not connected to broker
    MQTT_STATUS_CONNECTING,        ///< Attempting to connect
    MQTT_STATUS_CONNECTED,         ///< Successfully connected
    MQTT_STATUS_ERROR              ///< Connection error
} mqtt_status_t;

/**
 * @brief Callback function type for incoming MQTT messages
 * 
 * @param topic The topic the message was received on (e.g., "SmartLove/ABC123/in")
 * @param topic_len Length of the topic string
 * @param data The message payload
 * @param data_len Length of the message payload
 * @param user_data User data pointer provided during callback registration
 */
typedef void (*mqtt_message_callback_t)(const char *topic, int topic_len,
                                       const char *data, int data_len,
                                       void *user_data);

/**
 * @brief Callback function type for MQTT connection status changes
 * 
 * @param status New connection status
 * @param user_data User data pointer provided during callback registration
 */
typedef void (*mqtt_status_callback_t)(mqtt_status_t status, void *user_data);

/**
 * @brief Initialize the MQTT client
 * 
 * This function must be called before any other MQTT operations.
 * It reads the chip ID and sets up the MQTT topics.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_init(void);

/**
 * @brief Start the MQTT client and connect to broker
 * 
 * Should be called after WiFi is connected (STA mode).
 * The client will automatically reconnect if connection is lost.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_start(void);

/**
 * @brief Stop the MQTT client and disconnect from broker
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_stop(void);

/**
 * @brief Check if MQTT client is connected
 * 
 * @return true if connected, false otherwise
 */
bool mqtt_client_is_connected(void);

/**
 * @brief Get current MQTT connection status
 * 
 * @return Current status
 */
mqtt_status_t mqtt_client_get_status(void);

/**
 * @brief Register callback for incoming messages
 * 
 * The callback will be called whenever a message is received on the
 * subscribed topic (SmartLove/<chipID>/in).
 * 
 * @param callback Callback function pointer
 * @param user_data Optional user data passed to callback
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_register_message_callback(mqtt_message_callback_t callback,
                                               void *user_data);

/**
 * @brief Register callback for connection status changes
 * 
 * @param callback Callback function pointer
 * @param user_data Optional user data passed to callback
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_register_status_callback(mqtt_status_callback_t callback,
                                              void *user_data);

/**
 * @brief Send a message to the outgoing topic
 * 
 * Publishes a message to SmartLove/<chipID>/out.
 * 
 * @param message Message payload (null-terminated string)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_send(const char *message);

/**
 * @brief Send a message with specified length to the outgoing topic
 * 
 * Use this for binary data or when message is not null-terminated.
 * 
 * @param data Message payload
 * @param len Length of the payload
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_send_data(const char *data, int len);

/**
 * @brief Publish a message to a custom topic
 * 
 * Publishes to a fully qualified topic (not using the default prefix).
 * 
 * @param topic Full topic path
 * @param message Message payload
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_publish(const char *topic, const char *message);

/**
 * @brief Get the chip ID string
 * 
 * Returns the unique chip ID used in MQTT topics.
 * 
 * @param buffer Buffer to store the chip ID string
 * @param buffer_size Size of the buffer
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_get_chip_id(char *buffer, size_t buffer_size);

/**
 * @brief Get the full outgoing topic path
 * 
 * Returns: SmartLove/<chipID>/out
 * 
 * @param buffer Buffer to store the topic string
 * @param buffer_size Size of the buffer
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_get_out_topic(char *buffer, size_t buffer_size);

/**
 * @brief Get the full incoming topic path
 * 
 * Returns: SmartLove/<chipID>/in
 * 
 * @param buffer Buffer to store the topic string
 * @param buffer_size Size of the buffer
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_get_in_topic(char *buffer, size_t buffer_size);

/**
 * @brief Deinitialize the MQTT client
 * 
 * Cleans up all resources. Call this before program exit.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_client_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // SMARTLOVE_MQTT_CLIENT_H
