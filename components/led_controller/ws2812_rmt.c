/**
 * @file ws2812_rmt.c
 * @brief WS2812B LED Driver using RMT peripheral (IDF 4.x compatible)
 */

#include "ws2812_rmt.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "ws2812";

// WS2812 timing (in RMT ticks at 80MHz / 80 = 1MHz = 1μs per tick)
#define WS2812_T0H    28  // 0.35μs = ~28 ticks
#define WS2812_T0L    72  // 0.90μs = ~72 ticks  
#define WS2812_T1H    72  // 0.90μs = ~72 ticks
#define WS2812_T1L    28  // 0.35μs = ~28 ticks

/**
 * @brief WS2812 strip structure
 */
struct ws2812_strip_t {
    uint8_t gpio_num;
    uint16_t led_count;
    uint8_t rmt_channel;
    uint8_t *led_buffer;  // GRB format: 3 bytes per LED
};

/**
 * @brief Convert bit to RMT item
 */
static inline void ws2812_bit_to_rmt(uint8_t bit, rmt_item32_t *item)
{
    if (bit) {
        // Bit 1: High for T1H, Low for T1L
        item->level0 = 1;
        item->duration0 = WS2812_T1H;
        item->level1 = 0;
        item->duration1 = WS2812_T1L;
    } else {
        // Bit 0: High for T0H, Low for T0L
        item->level0 = 1;
        item->duration0 = WS2812_T0H;
        item->level1 = 0;
        item->duration1 = WS2812_T0L;
    }
}

ws2812_handle_t ws2812_init(uint8_t gpio_num, uint16_t led_count, uint8_t rmt_channel)
{
    // Allocate strip structure
    struct ws2812_strip_t *strip = malloc(sizeof(struct ws2812_strip_t));
    if (strip == NULL) {
        ESP_LOGE(TAG, "Failed to allocate strip structure");
        return NULL;
    }

    strip->gpio_num = gpio_num;
    strip->led_count = led_count;
    strip->rmt_channel = rmt_channel;

    // Allocate LED buffer (GRB: 3 bytes per LED)
    strip->led_buffer = calloc(led_count * 3, sizeof(uint8_t));
    if (strip->led_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        free(strip);
        return NULL;
    }

    // Configure RMT
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = rmt_channel,
        .gpio_num = gpio_num,
        .clk_div = 80,  // 80MHz / 80 = 1MHz = 1μs per tick
        .mem_block_num = 1,
        .tx_config = {
            .carrier_en = false,
            .loop_en = false,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .idle_output_en = true,
        }
    };

    esp_err_t ret = rmt_config(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure RMT: %s", esp_err_to_name(ret));
        free(strip->led_buffer);
        free(strip);
        return NULL;
    }

    ret = rmt_driver_install(rmt_channel, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install RMT driver: %s", esp_err_to_name(ret));
        free(strip->led_buffer);
        free(strip);
        return NULL;
    }

    ESP_LOGI(TAG, "WS2812 initialized: GPIO %d, %d LEDs, RMT channel %d",
             gpio_num, led_count, rmt_channel);

    return strip;
}

void ws2812_deinit(ws2812_handle_t strip)
{
    if (strip == NULL) {
        return;
    }

    rmt_driver_uninstall(strip->rmt_channel);
    free(strip->led_buffer);
    free(strip);
}

esp_err_t ws2812_set_pixel(ws2812_handle_t strip, uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (strip == NULL || index >= strip->led_count) {
        return ESP_ERR_INVALID_ARG;
    }

    // WS2812 uses GRB order
    uint16_t offset = index * 3;
    strip->led_buffer[offset + 0] = g;
    strip->led_buffer[offset + 1] = r;
    strip->led_buffer[offset + 2] = b;

    return ESP_OK;
}

esp_err_t ws2812_refresh(ws2812_handle_t strip)
{
    if (strip == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Calculate number of bits to send
    uint32_t total_bytes = strip->led_count * 3;
    uint32_t total_bits = total_bytes * 8;

    // Allocate RMT items (each bit = 1 RMT item)
    rmt_item32_t *items = malloc(total_bits * sizeof(rmt_item32_t));
    if (items == NULL) {
        ESP_LOGE(TAG, "Failed to allocate RMT items");
        return ESP_ERR_NO_MEM;
    }

    // Convert each bit to RMT item
    uint32_t item_index = 0;
    for (uint32_t byte_index = 0; byte_index < total_bytes; byte_index++) {
        uint8_t byte = strip->led_buffer[byte_index];
        for (int bit = 7; bit >= 0; bit--) {
            ws2812_bit_to_rmt((byte >> bit) & 1, &items[item_index++]);
        }
    }

    // Send data via RMT
    esp_err_t ret = rmt_write_items(strip->rmt_channel, items, total_bits, true);
    
    free(items);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send RMT data: %s", esp_err_to_name(ret));
        return ret;
    }

    // Wait for transmission to complete
    ret = rmt_wait_tx_done(strip->rmt_channel, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT transmission timeout");
        return ret;
    }

    return ESP_OK;
}

esp_err_t ws2812_clear(ws2812_handle_t strip)
{
    if (strip == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(strip->led_buffer, 0, strip->led_count * 3);
    return ws2812_refresh(strip);
}
