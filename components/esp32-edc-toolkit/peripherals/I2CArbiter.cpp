/**
 * @file I2CArbiter.cpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2022-05-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "I2CArbiter.hpp"

#define LOGTAG "I2CArbiter"

//necessary because i use two taskes asynchronously to communicate with the ADC on the same device
SemaphoreHandle_t I2CArbiter::I2CMutex = NULL;

I2CArbiter::I2CArbiter() : I2C() {
    I2CMutex = xSemaphoreCreateMutex();
    init(0x00, SMARTFIT_I2C_SDA_NUM, SMARTFIT_I2C_SCL_NUM, SMARTFIT_I2C_SPEED, I2C_NUM_1);
}

I2CArbiter::~I2CArbiter(){
    vSemaphoreDelete(I2CMutex);
}

esp_err_t I2CArbiter::acquireBus(){
    if(xSemaphoreTake(I2CMutex, portMAX_DELAY) == pdTRUE){
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t I2CArbiter::releaseBus(){
    if(xSemaphoreGive(I2CMutex) == pdTRUE){
        return ESP_OK;
    }
    return ESP_FAIL;
}