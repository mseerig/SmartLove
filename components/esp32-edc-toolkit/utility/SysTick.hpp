/**
 * @file SysTick.hpp
 * @author your name (you@domain.com)
 * @brief Simple Class providing a configurable Systick Timer
 * @version 0.1
 * @date 2019-01-29
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef COMPONENTS_UTILITY_SYSTICK_H_
#define COMPONENTS_UTILITY_SYSTICK_H_

#include <stdint.h>
#include "esp_timer.h"


class SysTick {
public:
	SysTick();
	~SysTick();

	static uint_fast32_t getSysTick(void) { return m_systick;}

private:
	static esp_timer_handle_t m_systickTimerHandle;
	static volatile uint_fast32_t m_systick;
	static void systickHandler(void* arg);

};

#endif