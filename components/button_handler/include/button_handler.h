/**
 * @file button_handler.h
 * @brief Button Handler with Press Duration Tracking
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "esp_err.h"
#include <stdint.h>
#include "smartlove_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Configuration (from smartlove_config.h)
// ============================================================================

#define BUTTON_GPIO_PIN         SMARTLOVE_GPIO_BUTTON
#define BUTTON_DEBOUNCE_MS      SMARTLOVE_BUTTON_DEBOUNCE_MS
#define BUTTON_ACTIVE_LEVEL     (SMARTLOVE_BUTTON_ACTIVE_LOW ? 0 : 1)

// ============================================================================
// Public Functions
// ============================================================================

/**
 * @brief Initialize button handler
 * 
 * Sets up GPIO input with pull-up/pull-down and interrupt handling.
 * Automatically sends MQTT message with press duration when button is released.
 * 
 * @return ESP_OK on success
 */
esp_err_t button_handler_init(void);

/**
 * @brief Deinitialize button handler
 */
void button_handler_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_HANDLER_H
