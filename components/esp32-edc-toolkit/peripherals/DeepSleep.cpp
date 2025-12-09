/*
 * DeepSleep.cpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#include "DeepSleep.hpp"

#include "esp_sleep.h"
#include "esp_log.h"

static char TAG[] =  "DeepSleep";

/**
 * @brief Get the source which caused wakeup from sleep
 *
 * @return wakeup cause, or ESP_DEEP_SLEEP_WAKEUP_UNDEFINED if reset happened for reason other than deep sleep wakeup
 */
esp_sleep_wakeup_cause_t DeepSleep::getWakeupCause(void){
	return ::esp_sleep_get_wakeup_cause();
}

/**
 * @brief Enable wakeup by timer
 * @param time_in_ms  time before wakeup, in milliseconds
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if value is out of range (TBD)
 */
esp_err_t DeepSleep::setWakeupTimer(uint64_t time_in_ms){
	return ::esp_sleep_enable_timer_wakeup(time_in_ms*1000);
}

/**
 * @brief Disable wakeup source (timer)
 *
 * This function is used to deactivate wake up trigger for source
 * defined as parameter of the function.
 *
 * @note This function does not modify wake up configuration in RTC.
 *       It will be performed in esp_sleep_start function.
 *
 * See docs/sleep-modes.rst for details.
 *
 * @param void
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if trigger was not active
 */
esp_err_t DeepSleep::disableWakeUpTimer(void){
	return ::esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
}

/**
 * @brief Enter deep sleep with the configured wakeup options
 *
 * This function does not return.
 */
void DeepSleep::enterSleep(void){
	ESP_LOGI(TAG, "Enter deep sleep!");
	::esp_deep_sleep_start();
}

/**
 * @brief Enter deep-sleep mode
 *
 * The device will automatically wake up after the deep-sleep time
 * Upon waking up, the device calls deep sleep wake stub, and then proceeds
 * to load application.
 *
 * Call to this function is equivalent to a call to esp_deep_sleep_enable_timer_wakeup
 * followed by a call to esp_deep_sleep_start.
 *
 * esp_deep_sleep does not shut down WiFi, BT, and higher level protocol
 * connections gracefully.
 * Make sure relevant WiFi and BT stack functions are called to close any
 * connections and deinitialize the peripherals. These include:
 *     - esp_bluedroid_disable
 *     - esp_bt_controller_disable
 *     - esp_wifi_stop
 *
 * This function does not return.
 *
 * @param time_in_ms  deep-sleep time, unit: millisecond
 */
void DeepSleep::sleep(uint64_t time_in_ms){
	ESP_LOGI(TAG, "Enter deep sleep for %d milliseconds!", (int)time_in_ms);
	::esp_deep_sleep(time_in_ms*1000);
}