/**
 * @file I2CArbiter.hpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef I2CARBITER_HPP
#define I2CARBITER_HPP

#include "I2C.hpp"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "Definitions.hpp"

class I2CArbiter : public I2C {
    public:
        I2CArbiter();
        ~I2CArbiter();
        esp_err_t acquireBus();
        esp_err_t releaseBus();
    private:
        static SemaphoreHandle_t I2CMutex;
};

#endif