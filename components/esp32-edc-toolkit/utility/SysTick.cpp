/**
 * @file SysTick.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2019-01-29
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "SysTick.hpp"
#include "esp_log.h"

volatile uint_fast32_t SysTick::m_systick = 0;
esp_timer_handle_t SysTick::m_systickTimerHandle;

void SysTick::systickHandler(void* arg){
	SysTick::m_systick += 10;
    ESP_ERROR_CHECK(esp_timer_start_once(SysTick::m_systickTimerHandle, 10 * 1000));
}

SysTick::SysTick() {
    esp_timer_create_args_t sysTickTimerArgs = {
        .callback = reinterpret_cast<esp_timer_cb_t>(&SysTick::systickHandler),
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "SysTick",
        .skip_unhandled_events = false
    };

	//Create the Timer
    ESP_ERROR_CHECK(esp_timer_create(&sysTickTimerArgs, &SysTick::m_systickTimerHandle));

    //Start the Timer
    ESP_ERROR_CHECK(esp_timer_start_once(SysTick::m_systickTimerHandle, 10 * 1000));
}

SysTick::~SysTick() {
    ESP_ERROR_CHECK(esp_timer_stop(SysTick::m_systickTimerHandle));
    ESP_ERROR_CHECK(esp_timer_delete(SysTick::m_systickTimerHandle));
}