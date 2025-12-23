/**
 * @file led_controller.h
 * @brief WS2812B LED Controller with MQTT JSON control
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>
#include "smartlove_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Configuration (from smartlove_config.h)
// ============================================================================

#define LED_GPIO_PIN            SMARTLOVE_GPIO_LED_STRIP
#define LED_STRIP_LENGTH        SMARTLOVE_LED_COUNT
#define LED_MAX_BRIGHTNESS      SMARTLOVE_LED_MAX_BRIGHTNESS
#define LED_RMT_CHANNEL         SMARTLOVE_LED_RMT_CHANNEL

// ============================================================================
// Types
// ============================================================================

/**
 * @brief RGB Color structure
 */
typedef struct {
    uint8_t r;  ///< Red component (0-255)
    uint8_t g;  ///< Green component (0-255)
    uint8_t b;  ///< Blue component (0-255)
} led_rgb_t;

/**
 * @brief LED Animation types
 */
typedef enum {
    LED_ANIM_NONE = 0,      ///< No animation, static color
    LED_ANIM_BLINK,         ///< Blink animation (0.5s interval)
    LED_ANIM_FADE           ///< Fade to new color
} led_animation_t;

/**
 * @brief LED Controller state
 */
typedef struct {
    uint8_t intensity;           ///< Brightness (0-255)
    led_rgb_t color;             ///< Current color
    led_rgb_t fade_start;        ///< Start color for fade
    led_rgb_t fade_target;       ///< Target color for fade
    uint32_t fade_time_ms;       ///< Fade duration in ms
    uint32_t fade_elapsed_ms;    ///< Fade elapsed time
    led_animation_t animation;   ///< Current animation
    bool is_on;                  ///< LED strip on/off state
} led_state_t;

// ============================================================================
// API Functions
// ============================================================================

/**
 * @brief Initialize LED controller
 * 
 * Initializes the WS2812B LED strip on the configured GPIO pin
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_init(void);

/**
 * @brief Deinitialize LED controller
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_deinit(void);

/**
 * @brief Set LED intensity (brightness)
 * 
 * @param intensity Brightness level (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_set_intensity(uint8_t intensity);

/**
 * @brief Set LED color
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set LED animation
 * 
 * @param animation Animation type
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_set_animation(led_animation_t animation);

/**
 * @brief Fade to a new color over a given time
 *
 * @param r Target red
 * @param g Target green
 * @param b Target blue
 * @param duration_ms Fade duration in ms
 * @return ESP_OK on success
 */
esp_err_t led_controller_fade_to(uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms);

/**
 * @brief Process JSON command
 * 
 * Accepts JSON in format:
 * {
 *   "intensity": 0-255,     // Optional: brightness
 *   "color": {              // Optional: RGB color
 *     "r": 0-255,
 *     "g": 0-255,
 *     "b": 0-255
 *   },
 *   "show": "BLINK"         // Optional: animation code
 * }
 * 
 * @param json_str JSON command string
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_process_json(const char *json_str);

/**
 * @brief Turn LEDs on
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_on(void);

/**
 * @brief Turn LEDs off
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_off(void);

/**
 * @brief Get current LED state
 * 
 * @param state Pointer to state structure to fill
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_controller_get_state(led_state_t *state);

#ifdef __cplusplus
}
#endif

#endif // LED_CONTROLLER_H
