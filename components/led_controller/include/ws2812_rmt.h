/**
 * @file ws2812_rmt.h
 * @brief WS2812B LED Driver using RMT peripheral (IDF 4.x compatible)
 */

#ifndef WS2812_RMT_H
#define WS2812_RMT_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WS2812 LED strip handle
 */
typedef struct ws2812_strip_t* ws2812_handle_t;

/**
 * @brief Initialize WS2812 LED strip
 * 
 * @param gpio_num GPIO pin number
 * @param led_count Number of LEDs
 * @param rmt_channel RMT channel to use
 * @return Handle to LED strip or NULL on error
 */
ws2812_handle_t ws2812_init(uint8_t gpio_num, uint16_t led_count, uint8_t rmt_channel);

/**
 * @brief Deinitialize WS2812 LED strip
 * 
 * @param strip LED strip handle
 */
void ws2812_deinit(ws2812_handle_t strip);

/**
 * @brief Set pixel color (GRB order for WS2812)
 * 
 * @param strip LED strip handle
 * @param index LED index (0-based)
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return ESP_OK on success
 */
esp_err_t ws2812_set_pixel(ws2812_handle_t strip, uint16_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Refresh LED strip (send data)
 * 
 * @param strip LED strip handle
 * @return ESP_OK on success
 */
esp_err_t ws2812_refresh(ws2812_handle_t strip);

/**
 * @brief Clear all LEDs (set to black)
 * 
 * @param strip LED strip handle
 * @return ESP_OK on success
 */
esp_err_t ws2812_clear(ws2812_handle_t strip);

#ifdef __cplusplus
}
#endif

#endif // WS2812_RMT_H
