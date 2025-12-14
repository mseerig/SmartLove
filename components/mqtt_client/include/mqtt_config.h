/**
 * @file mqtt_config.h
 * @brief MQTT Configuration - Central place for all MQTT settings
 */

#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MQTT Broker Configuration
// ============================================================================

/**
 * @brief MQTT Broker Address
 * Format: mqtt://hostname or mqtts://hostname for TLS
 */
#define MQTT_BROKER_URI         "2c219cce853346499b53f9a2efcce644.s1.eu.hivemq.cloud"

/**
 * @brief MQTT Broker Port
 * Default: 1883 (unencrypted), 8883 (TLS)
 */
#define MQTT_BROKER_PORT        8883

/**
 * @brief MQTT Username (leave empty if not required)
 */
#define MQTT_USERNAME           "SmartLove"

/**
 * @brief MQTT Password (leave empty if not required)
 */
#define MQTT_PASSWORD           "Y9u9ys#nIKBF8Y^c"

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
 */
#define MQTT_USE_TLS            false

/**
 * @brief Skip certificate verification (not recommended for production)
 */
#define MQTT_SKIP_CERT_VERIFY   false

// If MQTT_USE_TLS is true, you can provide certificates here:
// extern const uint8_t mqtt_broker_cert_pem_start[] asm("_binary_mqtt_broker_cert_pem_start");
// extern const uint8_t mqtt_broker_cert_pem_end[]   asm("_binary_mqtt_broker_cert_pem_end");

#ifdef __cplusplus
}
#endif

#endif // MQTT_CONFIG_H
