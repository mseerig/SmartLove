/**
 * @file TimerInterface.h
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief interface class for the Timer class; used to allow for dependency injection in test
 * @version 0.1
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BSP_TIMERINTERFACE_H_
#define BSP_TIMERINTERFACE_H_

#include <limits>
#include <cstdint>

/**
 * @brief - pure virtual function description to force implementation in both production code and mock 
 *        - declaration and defintion of time calculation functions which can be used in both prod and test
 */
class TimerInterface{
public:
    typedef uint_fast32_t tick_type;
    static tick_type milliseconds(const tick_type& value_milliseconds) { return value_milliseconds; }
    static tick_type seconds     (const tick_type& value_seconds     ) { return static_cast<tick_type>(1000UL) * milliseconds(value_seconds     ); }
    static tick_type minutes     (const tick_type& value_minutes     ) { return static_cast<tick_type>(  60UL) * seconds     (value_minutes     ); }
    static tick_type hours       (const tick_type& value_hours       ) { return static_cast<tick_type>(  60UL) * minutes     (value_hours       ); }
    static tick_type days        (const tick_type& value_days        ) { return static_cast<tick_type>(  24UL) * hours       (value_days        ); }
    static tick_type weeks       (const tick_type& value_weeks       ) { return static_cast<tick_type>(   7UL) * days        (value_weeks       ); }
    
    virtual ~TimerInterface() = default;
    virtual bool timeout() const = 0;
    virtual void start_relative(const tick_type& tick_value) = 0;
};

#endif
