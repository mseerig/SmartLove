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

#include "EventLog.hpp"
#include "System.hpp"

#include <fstream>
#include <algorithm>
#include "esp_log.h"

static char LOGTAG[] = "EventLog";

EventLog::EventLog(std::string filePath, uint16_t limit):
    m_filePath(filePath), 
    m_limit(limit){

    ESP_LOGI(LOGTAG, "Starting");
    ESP_LOGD(LOGTAG, "Using File: %s", m_filePath.c_str());

    m_counter = getLogCount();
    ESP_LOGI(LOGTAG, "Eventlog has length of %d/%d", m_counter, m_limit);

}

EventLog::~EventLog(){
    // -.-
}

// add event by code to logfile (timestamp added internally)
esp_err_t EventLog::push(Event event, State state){
    return push((int)event, state);
}

// add event by code to logfile (timestamp added internally)
esp_err_t EventLog::push(int event, State state){

    if(m_counter >= m_limit){
        clear();
    }
    m_counter++;

    // append event to file <timestamp>, <event>, <state>
    std::ofstream outfile;
    outfile.open(m_filePath, std::ios_base::app); // append instead of overwrite
    outfile << System::getTime() << ", " << event << ", " << (int)state << std::endl;
    outfile.close();
    ESP_LOGD(LOGTAG, "Log Event: %d State: %d", event, (int)state);

    // Store the new state, if this was more critical then befor
    if (state > m_state) m_state = state;

    return ESP_OK;
}

/**
 * @brief 
 * Cleares the whole log file and add a "SYSTEM_RESET_ALERT".
 * 
 * @return esp_err_t 
 */
esp_err_t EventLog::clear(){
    ESP_LOGI(LOGTAG, "Eventlog file cleared!");
    m_counter=0;
    m_state = State::RUNNING;

    std::ofstream outfile;
    outfile.open(m_filePath, std::ios::trunc); // override instead
    outfile << "";
    outfile.close();

    return push(Event::SYSTEM_RESET_ALERT, State::INFO);
}

/**
 * @brief Set system State to RUNNING
 * 
 * @return esp_err_t 
 */
esp_err_t EventLog::resetStatus(){
    m_state = State::RUNNING;
    return push(Event::SYSTEM_RESET_ALERT, State::INFO);
}

/**
 * @brief Give the last (highest) system state form enum class State.
 * 
 * @return State {RUNNING, INFO, WARNING, ERROR}
 */
EventLog::State EventLog::getStatus(){
    return m_state;
}


/**
 * @brief Count lines of File
 * 
 * @return esp_err_t 
 */
uint16_t EventLog::getLogCount(){
    std::ifstream inFile(m_filePath); 
    uint16_t num = std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n');
    return num;
}

void EventLog::eraseFront(){
    ESP_LOGI(LOGTAG, "erase Front!");

    /*
    What you are asking for is only possible if the searched string and the replacement string are the 
    same length, or if the searched string is at the very end of the file. Only then are you able to 
    modify the source file directly. Otherwise, if the two strings are different lengths, or the searched 
    string is in the front/middle of the file, then it is simply not possible to modify the source file, 
    you must create a separate temp file and then replace the source file when ready.
https://stackoverflow.com/questions/22509234/how-to-replace-a-string-in-a-text-file-without-copying-the-text-into-new-file
    */
}