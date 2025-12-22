/**
 * @file led_controller.c
 * @brief WS2812B LED Controller Implementation
 */

#include "led_controller.h"
#include "ws2812_rmt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "led_controller";

// LED Controller State - initialized with config defaults
static led_state_t s_led_state = {
    .is_on = SMARTLOVE_LED_DEFAULT_ON,
    .intensity = SMARTLOVE_LED_DEFAULT_BRIGHTNESS,
    .color = {
        .r = SMARTLOVE_LED_DEFAULT_COLOR_R,
        .g = SMARTLOVE_LED_DEFAULT_COLOR_G,
        .b = SMARTLOVE_LED_DEFAULT_COLOR_B
    },
    .animation = SMARTLOVE_LED_DEFAULT_ANIMATION
};

static ws2812_handle_t s_led_strip = NULL;
static TaskHandle_t s_animation_task_handle = NULL;
static bool s_initialized = false;

// ============================================================================
// Private Functions
// ============================================================================

/**
 * @brief Apply current color and intensity to all LEDs
 */
static void apply_leds(void)
{
    if (!s_initialized || s_led_strip == NULL) {
        return;
    }

    if (!s_led_state.is_on) {
        // Turn off all LEDs
        ws2812_clear(s_led_strip);
        return;
    }

    // Calculate scaled RGB values based on intensity
    uint8_t r = (s_led_state.color.r * s_led_state.intensity) / 255;
    uint8_t g = (s_led_state.color.g * s_led_state.intensity) / 255;
    uint8_t b = (s_led_state.color.b * s_led_state.intensity) / 255;

    // Set all LEDs to the same color
    for (int i = 0; i < LED_STRIP_LENGTH; i++) {
        ws2812_set_pixel(s_led_strip, i, r, g, b);
    }

    // Refresh the strip
    ws2812_refresh(s_led_strip);
}

/**
 * @brief Animation task for LED effects
 */
static void animation_task(void *pvParameters)
{
    bool blink_state = false;
    
    ESP_LOGI(TAG, "Animation task started");

    while (1) {
        if (s_led_state.animation == LED_ANIM_BLINK) {
            // Toggle blink state
            blink_state = !blink_state;
            
            if (blink_state) {
                // Turn LEDs on with current color
                bool was_on = s_led_state.is_on;
                s_led_state.is_on = true;
                apply_leds();
                s_led_state.is_on = was_on;
            } else {
                // Turn LEDs off
                ws2812_clear(s_led_strip);
            }
            
            vTaskDelay(pdMS_TO_TICKS(500)); // 500ms interval
        } else {
            // No animation, just wait
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

/**
 * @brief Start animation task
 */
static void start_animation_task(void)
{
    if (s_animation_task_handle != NULL) {
        return; // Task already running
    }

    BaseType_t ret = xTaskCreate(
        animation_task,
        "led_anim",
        2048,
        NULL,
        5,
        &s_animation_task_handle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create animation task");
        s_animation_task_handle = NULL;
    } else {
        ESP_LOGI(TAG, "Animation task created");
    }
}

/**
 * @brief Stop animation task
 */
static void stop_animation_task(void)
{
    if (s_animation_task_handle != NULL) {
        vTaskDelete(s_animation_task_handle);
        s_animation_task_handle = NULL;
        ESP_LOGI(TAG, "Animation task stopped");
    }
}

// ============================================================================
// Public Functions
// ============================================================================

esp_err_t led_controller_init(void)
{
    if (s_initialized) {
        ESP_LOGW(TAG, "LED controller already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing LED controller (GPIO %d, %d LEDs)", 
             LED_GPIO_PIN, LED_STRIP_LENGTH);

    // Initialize WS2812 strip with RMT
    s_led_strip = ws2812_init(LED_GPIO_PIN, LED_STRIP_LENGTH, 0);
    if (s_led_strip == NULL) {
        ESP_LOGE(TAG, "Failed to initialize WS2812 strip");
        return ESP_FAIL;
    }

    // Clear LEDs initially
    ws2812_clear(s_led_strip);

    s_initialized = true;
    ESP_LOGI(TAG, "LED controller initialized successfully");
    
    return ESP_OK;
}

esp_err_t led_controller_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing LED controller");

    // Stop animation if running
    stop_animation_task();

    // Clear and free LED strip
    if (s_led_strip != NULL) {
        ws2812_clear(s_led_strip);
        ws2812_deinit(s_led_strip);
        s_led_strip = NULL;
    }

    s_initialized = false;
    return ESP_OK;
}

esp_err_t led_controller_set_intensity(uint8_t intensity)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    s_led_state.intensity = intensity;
    
    // Auto ON/OFF based on intensity
    if (intensity == 0) {
        s_led_state.is_on = false;
        ESP_LOGI(TAG, "Intensity set to 0 - LEDs OFF");
    } else {
        s_led_state.is_on = true;
        ESP_LOGI(TAG, "Intensity set to %d - LEDs ON", intensity);
    }

    // Apply changes if no animation
    if (s_led_state.animation == LED_ANIM_NONE) {
        apply_leds();
    }

    return ESP_OK;
}

esp_err_t led_controller_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    s_led_state.color.r = r;
    s_led_state.color.g = g;
    s_led_state.color.b = b;

    ESP_LOGI(TAG, "Color set to RGB(%d, %d, %d)", r, g, b);

    // Auto-enable LEDs when color is set (if intensity > 0)
    if (s_led_state.intensity > 0 && !s_led_state.is_on) {
        s_led_state.is_on = true;
        ESP_LOGI(TAG, "LEDs auto-enabled");
    }

    // Apply changes if LEDs are on and no animation
    if (s_led_state.is_on && s_led_state.animation == LED_ANIM_NONE) {
        apply_leds();
    }

    return ESP_OK;
}

esp_err_t led_controller_set_animation(led_animation_t animation)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    s_led_state.animation = animation;

    if (animation == LED_ANIM_NONE) {
        // Stop animation task
        stop_animation_task();
        
        // Restore static color if LEDs are on
        if (s_led_state.is_on) {
            apply_leds();
        }
        
        ESP_LOGI(TAG, "Animation stopped");
    } else {
        // Start animation task if not running
        start_animation_task();
        ESP_LOGI(TAG, "Animation set to: %d", animation);
    }

    return ESP_OK;
}

esp_err_t led_controller_on(void)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    s_led_state.is_on = true;
    ESP_LOGI(TAG, "LEDs turned ON");

    // Apply changes if no animation is running
    if (s_led_state.animation == LED_ANIM_NONE) {
        apply_leds();
    }

    return ESP_OK;
}

esp_err_t led_controller_off(void)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    s_led_state.is_on = false;
    ESP_LOGI(TAG, "LEDs turned OFF");

    // Stop animation and clear LEDs
    stop_animation_task();
    ws2812_clear(s_led_strip);

    return ESP_OK;
}

esp_err_t led_controller_get_state(led_state_t *state)
{
    if (state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *state = s_led_state;
    return ESP_OK;
}

esp_err_t led_controller_process_json(const char *json_str)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (json_str == NULL) {
        ESP_LOGE(TAG, "NULL JSON string");
        return ESP_ERR_INVALID_ARG;
    }

    // Parse JSON
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_ERR_INVALID_ARG;
    }

    bool success = true;

    // Parse "intensity" (0-255)
    cJSON *intensity_item = cJSON_GetObjectItem(root, "intensity");
    if (intensity_item != NULL && cJSON_IsNumber(intensity_item)) {
        int intensity = intensity_item->valueint;
        if (intensity >= 0 && intensity <= 255) {
            led_controller_set_intensity((uint8_t)intensity);
        } else {
            ESP_LOGW(TAG, "Invalid intensity value: %d (must be 0-255)", intensity);
            success = false;
        }
    }

    // Parse "color" {r, g, b}
    cJSON *color_item = cJSON_GetObjectItem(root, "color");
    if (color_item != NULL && cJSON_IsObject(color_item)) {
        cJSON *r_item = cJSON_GetObjectItem(color_item, "r");
        cJSON *g_item = cJSON_GetObjectItem(color_item, "g");
        cJSON *b_item = cJSON_GetObjectItem(color_item, "b");

        if (cJSON_IsNumber(r_item) && cJSON_IsNumber(g_item) && cJSON_IsNumber(b_item)) {
            int r = r_item->valueint;
            int g = g_item->valueint;
            int b = b_item->valueint;

            if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                led_controller_set_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
            } else {
                ESP_LOGW(TAG, "Invalid RGB values: (%d, %d, %d)", r, g, b);
                success = false;
            }
        }
    }

    // Parse "show" (animation type)
    cJSON *show_item = cJSON_GetObjectItem(root, "show");
    if (show_item != NULL && cJSON_IsString(show_item)) {
        const char *show_str = show_item->valuestring;
        
        if (strcasecmp(show_str, "BLINK") == 0) {
            led_controller_set_animation(LED_ANIM_BLINK);
        } else if (strcasecmp(show_str, "NONE") == 0 || strcasecmp(show_str, "STATIC") == 0) {
            led_controller_set_animation(LED_ANIM_NONE);
        } else {
            ESP_LOGW(TAG, "Unknown animation: %s", show_str);
            success = false;
        }
    }

    cJSON_Delete(root);

    ESP_LOGI(TAG, "JSON processed: intensity=%d, RGB(%d,%d,%d), anim=%d",
             s_led_state.intensity, 
             s_led_state.color.r, s_led_state.color.g, s_led_state.color.b,
             s_led_state.animation);

    return success ? ESP_OK : ESP_ERR_INVALID_ARG;
}
