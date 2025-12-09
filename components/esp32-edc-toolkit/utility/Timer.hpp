/**
 * @file Timer.h
 * @author andre.lange
 *
 * @ingroup bsp
 * @brief Versatile timer class based on systick provided by \ref ClockModuleDriver
 */

#ifndef BSP_TIMER_H_
#define BSP_TIMER_H_

#include <limits>
#include <cstdint>
#include "SysTick.hpp"
#include "TimerInterface.h"

class TimerInterface;

class Timer : public TimerInterface{
public:

   static_assert(std::numeric_limits<tick_type>::is_signed == false, "the timer tick type must be unsigned"); 

   Timer() : my_tick(tick_type(my_now())) { }

   explicit Timer(const tick_type& tick_value) : my_tick(tick_type(my_now()) + tick_value) { }

   Timer(const Timer& other_timer) : my_tick(other_timer.my_tick) { }

   Timer& operator=(const Timer& other_timer)
   {
     my_tick = other_timer.my_tick;
     return *this;
   }

   void start_interval(const tick_type& tick_value)
   {
     my_tick += tick_value;
   }

   void start_relative(const tick_type& tick_value)
   {
     my_tick = static_cast<tick_type>(tick_type(my_now()) + tick_value);
   }

   bool timeout() const
   {
     const tick_type timer_mask = static_cast<tick_type>((1ULL << (std::numeric_limits<tick_type>::digits - 1)) - 1ULL);
     const tick_type delta      = static_cast<tick_type>(static_cast<tick_type>(my_now()) - my_tick);
     return (delta <= timer_mask);
   }

   void set_mark()
   {
     my_tick = static_cast<tick_type>(my_now());
   }

   tick_type get_ticks_since_mark() const
   {
     return static_cast<tick_type>(static_cast<tick_type>(my_now()) - my_tick);
   }

   static void blocking_delay(const tick_type& delay)
   {
     const Timer t_delay(delay);

     while(false == t_delay.timeout())
     {
    	 asm("nop");
     }
   }

 private:
   tick_type my_tick;

   static uint32_t my_now() { return SysTick::getSysTick(); }
};

#endif /* TIMER_H_ */
