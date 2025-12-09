/**
 * @file EventLog.hpp
 * @author Marcel Seerig (marcel@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-09-28
 * 
 * @copyright Copyright (c) 2022 EDC Electronic Design Chemnitz
 * 
 */


#include "esp_err.h"
#include <string>

#ifndef EVENT_LOG_HPP
#define EVENT_LOG_HPP


/**
 * @brief 
 * 
 */
class EventLog{

    public:

        /**
         * @brief Types of System status
         * 
         */
        enum class State{
            RUNNING = 0,
            INFO = 20,
            WARNING = 50,
            ERROR = 90,
        };

        /**
         * @brief List of defined events.
         * 
         */
        enum class Event{
            // 1xxx System
            SYSTEM_RESET_ALERT              = 1000,
            SYSTEM_POWER_UP                 = 1001,
            SYSTEM_RTC_NOT_WORKING          = 1002,

            // 2xxx Authenticator

            // 3xxx Network
            NETWORK_ETH_CONFIG_CHANGED      = 3001,
            NETWORK_WIFI_CONFIG_CHANGED     = 3002,

            // 4xxx Cloud
            CLOUD_CONFIG_CHANGED            = 4001,


            // 8xxx ExtensionController

            //...
        };

        EventLog(std::string filePath, uint16_t limit);
        ~EventLog();

        esp_err_t push(Event event, State state);       // add event by code to logfile (timestamp added internally)
        esp_err_t push(int event, State state);
        esp_err_t clear();                // clear the whole log file
        uint16_t getLogCount();           // give me the number of lines length of file

        esp_err_t resetStatus();          // reset the status flag
        State getStatus();                // get system status {RUNNING, INFO, WARNING, ERROR}:
                                          // -> returnes the highest last sys_status_t value till 
                                          //    resetStatus() was called. Like a industrial signal lamp

    private:
        State m_state{State::RUNNING};
        std::string m_filePath{""};
        uint16_t m_counter{0};
        uint16_t m_limit;

        void eraseFront();                  // erase oldest log message

};

#endif