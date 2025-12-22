/**
 * @file mqtt_config.h
 * @brief MQTT Configuration - references smartlove_config.h
 */

#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "smartlove_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MQTT Broker Configuration (from smartlove_config.h)
// ============================================================================

#define MQTT_BROKER_URI         SMARTLOVE_MQTT_BROKER_URI
#define MQTT_BROKER_PORT        SMARTLOVE_MQTT_BROKER_PORT
#define MQTT_USERNAME           SMARTLOVE_MQTT_USERNAME
#define MQTT_PASSWORD           SMARTLOVE_MQTT_PASSWORD

// ============================================================================
// Topic Configuration
// ============================================================================

/**
 * @brief Base topic prefix for all SmartLove devices
 * Will be combined with chip ID: SmartLove/<chipID>/...
 */
#define MQTT_TOPIC_PREFIX       "SmartLove"

/**
 * @brief Outgoing messages suffix
 * Full topic: SmartLove/<chipID>/out
 */
#define MQTT_TOPIC_OUT_SUFFIX   "out"

/**
 * @brief Incoming messages suffix
 * Full topic: SmartLove/<chipID>/in
 */
#define MQTT_TOPIC_IN_SUFFIX    "in"

/**
 * @brief Status topic suffix (optional for online/offline status)
 * Full topic: SmartLove/<chipID>/status
 */
#define MQTT_TOPIC_STATUS_SUFFIX "status"

// ============================================================================
// Connection Settings
// ============================================================================

/**
 * @brief Keep-alive interval in seconds
 * The client sends a ping to the broker every X seconds
 */
#define MQTT_KEEPALIVE_SECONDS  120

/**
 * @brief Reconnection timeout in milliseconds
 */
#define MQTT_RECONNECT_TIMEOUT_MS 5000

/**
 * @brief Maximum reconnection attempts (0 = infinite)
 */
#define MQTT_MAX_RECONNECT_ATTEMPTS 0

/**
 * @brief QoS level for published messages
 * 0 = At most once, 1 = At least once, 2 = Exactly once
 */
#define MQTT_QOS_LEVEL          1

/**
 * @brief Retain flag for published messages
 * true = Message is retained by broker, false = Not retained
 */
#define MQTT_RETAIN_FLAG        false

// ============================================================================
// Client Configuration
// ============================================================================

/**
 * @brief MQTT Client ID prefix
 * Will be combined with chip ID: smartlove_<chipID>
 */
#define MQTT_CLIENT_ID_PREFIX   "smartlove_"

/**
 * @brief Buffer size for incoming messages
 */
#define MQTT_BUFFER_SIZE        1024

/**
 * @brief Task stack size for MQTT
 */
#define MQTT_TASK_STACK_SIZE    4096

/**
 * @brief Task priority for MQTT
 */
#define MQTT_TASK_PRIORITY      5

// ============================================================================
// Last Will and Testament (LWT)
// ============================================================================

/**
 * @brief Enable LWT (Last Will and Testament)
 * Sends a message when client disconnects unexpectedly
 */
#define MQTT_LWT_ENABLED        true

/**
 * @brief LWT message payload
 */
#define MQTT_LWT_MESSAGE        "offline"

/**
 * @brief LWT QoS level
 */
#define MQTT_LWT_QOS            1

/**
 * @brief LWT retain flag
 */
#define MQTT_LWT_RETAIN         true
// ============================================================================
// TLS/SSL Configuration (if using mqtts://)
// ============================================================================

/**
 * @brief Enable TLS/SSL encryption
 * Set to true when using mqtts:// protocol
 * Currently disabled for public HiveMQ broker (mqtt://)
 */
#define MQTT_USE_TLS            false

/**
 * @brief Skip certificate verification (not recommended for production)
 * Set to true for testing with self-signed certificates
 */
#define MQTT_SKIP_CERT_VERIFY   true

// If MQTT_USE_TLS is true, you can provide certificates here:
// extern const uint8_t mqtt_broker_cert_pem_start[] asm("_binary_mqtt_broker_cert_pem_start");
// extern const uint8_t mqtt_broker_cert_pem_end[]   asm("_binary_mqtt_broker_cert_pem_end");

#ifdef __cplusplus
}
#endif

#endif // MQTT_CONFIG_H
