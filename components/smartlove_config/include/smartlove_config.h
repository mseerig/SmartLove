/**
 * @file smartlove_config.h
 * @brief Central configuration file for SmartLove project
 * 
 * All hardware pins, MQTT settings, WiFi configuration, and other
 * static parameters are defined here.
 */

#ifndef SMARTLOVE_CONFIG_H
#define SMARTLOVE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Project Information
// ============================================================================

#define SMARTLOVE_VERSION           "1.0.0"
#define SMARTLOVE_PROJECT_NAME      "SmartLove"

// ============================================================================
// WiFi Configuration
// ============================================================================

/**
 * @brief Enable WiFi configuration portal (AP mode + Captive Portal)
 * 
 * If set to 0, the device will only try to connect to default credentials
 * and will NOT start an AP/Captive Portal on failure
 */
#define SMARTLOVE_USE_WIFI_CONFIG_PORTAL    0

/**
 * @brief Default WiFi credentials
 * 
 * Used if no saved credentials exist or as fallback
 * Leave empty ("") to disable default credentials
 */
#define SMARTLOVE_DEFAULT_WIFI_SSID         "TP-Link_7D5C"
#define SMARTLOVE_DEFAULT_WIFI_PASSWORD     "82494476"

/**
 * @brief WiFi AP SSID format when in config mode
 * 
 * %04X will be replaced with last 4 hex digits of chip MAC
 */
#define SMARTLOVE_AP_SSID_FORMAT            "SmartLove-%04X"

/**
 * @brief WiFi AP password (empty = open network)
 */
#define SMARTLOVE_AP_PASSWORD               ""

/**
 * @brief WiFi connection retry attempts before fallback to AP
 */
#define SMARTLOVE_WIFI_MAX_RETRY            5

/**
 * @brief WiFi connection timeout per attempt (ms)
 */
#define SMARTLOVE_WIFI_TIMEOUT_MS           10000

// ============================================================================
// NVS (Non-Volatile Storage) Configuration
// ============================================================================

/**
 * @brief NVS namespace for WiFi credentials
 */
#define SMARTLOVE_NVS_WIFI_NAMESPACE        "wifi_config"

/**
 * @brief NVS namespace for application settings
 */
#define SMARTLOVE_NVS_APP_NAMESPACE         "smartlove"

/**
 * @brief NVS keys for WiFi credentials
 */
#define SMARTLOVE_NVS_KEY_SSID              "ssid"
#define SMARTLOVE_NVS_KEY_PASSWORD          "password"

// ============================================================================
// MQTT Configuration
// ============================================================================

/**
 * @brief MQTT Broker URI
 * 
 * Format: mqtt://hostname or mqtts://hostname for TLS
 */
#define SMARTLOVE_MQTT_BROKER_URI           "mqtt://broker.hivemq.com"
//#define SMARTLOVE_MQTT_BROKER_URI           "mqtts://2c219cce853346499b53f9a2efcce644.s1.eu.hivemq.cloud"

/**
 * @brief MQTT Broker Port
 */
#define SMARTLOVE_MQTT_BROKER_PORT          1883
//#define SMARTLOVE_MQTT_BROKER_PORT          8883

/**
 * @brief MQTT Username (empty if not required)
 */
#define SMARTLOVE_MQTT_USERNAME             ""
//#define SMARTLOVE_MQTT_USERNAME             "SmartLove"

/**
 * @brief MQTT Password (empty if not required)
 */
#define SMARTLOVE_MQTT_PASSWORD             ""
//#define SMARTLOVE_MQTT_PASSWORD             "oy2U!KtytbQeXp"

/**
 * @brief MQTT Topic prefix
 */
#define SMARTLOVE_MQTT_TOPIC_PREFIX         "SmartLove"

/**
 * @brief MQTT Topic suffix for outgoing messages
 */
#define SMARTLOVE_MQTT_TOPIC_OUT            "out"

/**
 * @brief MQTT Topic suffix for incoming messages
 */
#define SMARTLOVE_MQTT_TOPIC_IN             "in"

/**
 * @brief MQTT Topic suffix for status messages
 */
#define SMARTLOVE_MQTT_TOPIC_STATUS         "status"

/**
 * @brief MQTT QoS Level (0, 1, or 2)
 */
#define SMARTLOVE_MQTT_QOS                  1

/**
 * @brief MQTT Keep-Alive interval (seconds)
 */
#define SMARTLOVE_MQTT_KEEPALIVE_SEC        120

/**
 * @brief MQTT Client ID prefix
 */
#define SMARTLOVE_MQTT_CLIENT_ID_PREFIX     "smartlove_"

/**
 * @brief Enable TLS/SSL for MQTT
 */
#define SMARTLOVE_MQTT_USE_TLS              0

/**
 * @brief Skip TLS certificate verification (insecure!)
 */
#define SMARTLOVE_MQTT_SKIP_CERT_VERIFY     1

/**
 * @brief MQTT reconnect timeout (ms)
 */
#define SMARTLOVE_MQTT_RECONNECT_TIMEOUT_MS 5000

/**
 * @brief Enable MQTT Last Will and Testament
 */
#define SMARTLOVE_MQTT_LWT_ENABLED          1

/**
 * @brief MQTT LWT message
 */
#define SMARTLOVE_MQTT_LWT_MESSAGE          "offline"

// ============================================================================
// GPIO Pin Configuration
// ============================================================================

/**
 * @brief WS2812B LED strip data pin
 */
#define SMARTLOVE_GPIO_LED_STRIP            13

/**
 * @brief Number of LEDs in the strip
 */
#define SMARTLOVE_LED_COUNT                 2

/**
 * @brief Button/Switch input pin
 */
#define SMARTLOVE_GPIO_BUTTON               17

/**
 * @brief Button debounce time (ms)
 */
#define SMARTLOVE_BUTTON_DEBOUNCE_MS        50

/**
 * @brief Button active level (0 = active LOW with pull-up, 1 = active HIGH)
 */
#define SMARTLOVE_BUTTON_ACTIVE_LOW         1

// ============================================================================
// LED Controller Configuration
// ============================================================================

/**
 * @brief RMT channel for LED control
 */
#define SMARTLOVE_LED_RMT_CHANNEL           0

/**
 * @brief LED maximum brightness (0-255)
 */
#define SMARTLOVE_LED_MAX_BRIGHTNESS        255

/**
 * @brief LED default brightness on startup (0-255)
 */
#define SMARTLOVE_LED_DEFAULT_BRIGHTNESS    128

/**
 * @brief LED default color (RGB)
 */
#define SMARTLOVE_LED_DEFAULT_COLOR_R       255
#define SMARTLOVE_LED_DEFAULT_COLOR_G       255
#define SMARTLOVE_LED_DEFAULT_COLOR_B       255

/**
 * @brief LED default animation on startup
 * 0 = LED_ANIM_NONE
 * 1 = LED_ANIM_BLINK
 */
#define SMARTLOVE_LED_DEFAULT_ANIMATION     0

/**
 * @brief LED default ON state on startup
 * 0 = OFF, 1 = ON
 */
#define SMARTLOVE_LED_DEFAULT_ON            1

/**
 * @brief LED blink animation interval (ms)
 */
#define SMARTLOVE_LED_BLINK_INTERVAL_MS     500

// ============================================================================
// Web Server Configuration (Captive Portal)
// ============================================================================

/**
 * @brief Web server port
 */
#define SMARTLOVE_WEB_SERVER_PORT           80

/**
 * @brief DNS server port for captive portal
 */
#define SMARTLOVE_DNS_SERVER_PORT           53

/**
 * @brief Maximum connections for web server
 */
#define SMARTLOVE_WEB_MAX_CONNECTIONS       4

// ============================================================================
// System Configuration
// ============================================================================

/**
 * @brief System status/heartbeat interval (seconds)
 */
#define SMARTLOVE_STATUS_INTERVAL_SEC       10

/**
 * @brief MQTT heartbeat interval (seconds)
 */
#define SMARTLOVE_MQTT_HEARTBEAT_SEC        60

/**
 * @brief Enable debug logging
 */
#define SMARTLOVE_DEBUG_ENABLED             1

// ============================================================================
// Feature Flags
// ============================================================================

/**
 * @brief Enable MQTT client
 */
#define SMARTLOVE_FEATURE_MQTT              1

/**
 * @brief Enable LED controller
 */
#define SMARTLOVE_FEATURE_LED               1

/**
 * @brief Enable button handler
 */
#define SMARTLOVE_FEATURE_BUTTON            1

/**
 * @brief Enable captive portal (requires SMARTLOVE_USE_WIFI_CONFIG_PORTAL)
 */
#define SMARTLOVE_FEATURE_CAPTIVE_PORTAL    1

#ifdef __cplusplus
}
#endif

#endif // SMARTLOVE_CONFIG_H
