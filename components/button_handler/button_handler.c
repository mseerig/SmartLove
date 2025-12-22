/**
 * @file button_handler.c
 * @brief Button Handler Implementation with Press Duration Tracking
 */

#include "button_handler.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "smartlove_mqtt.h"
#include "smartlove_utils.h"
#include <stdio.h>

static const char *TAG = "button_handler";

// Button state tracking
static bool s_initialized = false;
static int64_t s_press_start_time = 0;
static bool s_button_pressed = false;
static QueueHandle_t s_button_event_queue = NULL;

// Button event types
typedef enum {
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_t;

/**
 * @brief GPIO interrupt handler
 */
static void IRAM_ATTR button_isr_handler(void* arg)
{
    button_event_t event;
    int level = gpio_get_level(BUTTON_GPIO_PIN);
    
    // For active LOW (button to GND with pull-up):
    // level == 0 (LOW) means pressed
    // level == 1 (HIGH) means released
    if (level == 0) {
        event = BUTTON_EVENT_PRESSED;
    } else {
        event = BUTTON_EVENT_RELEASED;
    }
    
    xQueueSendFromISR(s_button_event_queue, &event, NULL);
}

/**
 * @brief Button task to handle debouncing and MQTT messaging
 */
static void button_task(void *pvParameters)
{
    button_event_t event;
    TickType_t last_event_time = 0;
    
    ESP_LOGI(TAG, "Button task started");
    
    while (1) {
        if (xQueueReceive(s_button_event_queue, &event, portMAX_DELAY)) {
            TickType_t current_time = xTaskGetTickCount();
            
            // Debounce: ignore events too close together
            if ((current_time - last_event_time) < pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
                continue;
            }
            last_event_time = current_time;
            
            if (event == BUTTON_EVENT_PRESSED && !s_button_pressed) {
                // Button pressed
                s_press_start_time = esp_timer_get_time(); // microseconds
                s_button_pressed = true;
                ESP_LOGI(TAG, "Button pressed");
                
            } else if (event == BUTTON_EVENT_RELEASED && s_button_pressed) {
                // Button released - calculate duration
                int64_t press_end_time = esp_timer_get_time();
                int64_t duration_us = press_end_time - s_press_start_time;
                int64_t duration_ms = duration_us / 1000;
                
                s_button_pressed = false;
                
                ESP_LOGI(TAG, "Button released after %lld ms", duration_ms);
                
                // Send MQTT message with press duration
                char mqtt_msg[128];
                snprintf(mqtt_msg, sizeof(mqtt_msg),
                        "{\"event\":\"button_press\",\"duration_ms\":%lld,\"uptime\":%llu}",
                        duration_ms,
                        smartlove_get_uptime_ms() / 1000);
                
                mqtt_client_send(mqtt_msg);
                ESP_LOGI(TAG, "MQTT message sent: %s", mqtt_msg);
            }
        }
    }
}

esp_err_t button_handler_init(void)
{
    if (s_initialized) {
        ESP_LOGW(TAG, "Button handler already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing button handler (GPIO %d)", BUTTON_GPIO_PIN);
    
    // Create event queue
    s_button_event_queue = xQueueCreate(10, sizeof(button_event_t));
    if (s_button_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create button event queue");
        return ESP_FAIL;
    }
    
    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (BUTTON_ACTIVE_LEVEL == 0) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (BUTTON_ACTIVE_LEVEL == 1) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        vQueueDelete(s_button_event_queue);
        return ret;
    }
    
    // Install GPIO ISR service
    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(ret));
        vQueueDelete(s_button_event_queue);
        return ret;
    }
    
    // Add ISR handler for button GPIO
    ret = gpio_isr_handler_add(BUTTON_GPIO_PIN, button_isr_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler: %s", esp_err_to_name(ret));
        vQueueDelete(s_button_event_queue);
        return ret;
    }
    
    // Create button task
    BaseType_t task_ret = xTaskCreate(
        button_task,
        "button_task",
        3072,
        NULL,
        5,
        NULL
    );
    
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task");
        gpio_isr_handler_remove(BUTTON_GPIO_PIN);
        vQueueDelete(s_button_event_queue);
        return ESP_FAIL;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "Button handler initialized successfully");
    ESP_LOGI(TAG, "Button active level: %s", BUTTON_ACTIVE_LEVEL ? "HIGH" : "LOW");
    
    return ESP_OK;
}

void button_handler_deinit(void)
{
    if (!s_initialized) {
        return;
    }
    
    ESP_LOGI(TAG, "Deinitializing button handler");
    
    // Remove ISR handler
    gpio_isr_handler_remove(BUTTON_GPIO_PIN);
    
    // Delete queue
    if (s_button_event_queue != NULL) {
        vQueueDelete(s_button_event_queue);
        s_button_event_queue = NULL;
    }
    
    s_initialized = false;
}
