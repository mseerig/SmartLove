/**
 * @brief
 *
 * @file Definitions.hpp
 * @author Marcel Seerig
 * @date 2019-01-25
 */

/* IMPORTANT: ALL ELEMENTS NAMED " PROTOTYPE" WILL DELETED AUTOMATICALY IN RELEASE BUILD */

#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#define FIRMWARE_VERSION 	            "v0.1.0"
#define SMARTFIT_CORE_VERSION           "v1.1.8"
#define MODULE_TYPE 		            "smartFIT CORE"

#define FRONTEND_ADMIN_NAME             "smartFIT CORE" // default pass device ID backwards
#define FRONTEND_USER_NAME              "admin" // default pass device ID

#define DEFAULT_NAME                    "smartFIT CORE"
#define DEFAULT_LOCATION                ""

#define WIFI_AP_SSID 		            "smartFIT CORE Portal"
#define WIFI_AP_PASS 		            "12345678"

#define MQTT_TOPIC_PREFFIX              "smartFIT CORE"
#define MQTT_INFO_TOPIC_SUFFIX          "info"
#define MQTT_DATA_TOPIC_SUFFIX          "data"
#define MQTT_COMMAND_TOPIC_SUFFIX       "cmd"
#define MQTT_CONFIG_TOPIC_SUFFIX        "config"

#define MOSBUSTCP_HOLD_REG_NUM          9

#define EVENTLOG_LENGTH_MAX             40

#define SMARTFIT_I2C_SPEED	            100000
#define SMARTFIT_I2C_SDA_NUM            GPIO_NUM_45
#define SMARTFIT_I2C_SCL_NUM            GPIO_NUM_46

#define SMARTFIT_SPI_HOST_ID            SPI3_HOST
#define SMARTFIT_SPI_DMA_CHAN           0
#define SMARTFIT_SPI_CLK_PIN            GPIO_NUM_12
#define SMARTFIT_SPI_MOSI_PIN           GPIO_NUM_11
#define SMARTFIT_SPI_MISO_PIN           GPIO_NUM_13
#define SMARTFIT_FLASH_CS_PIN           GPIO_NUM_10
#define SMARTFIT_DAC_CS_PIN             GPIO_NUM_9
#define SMARTFIT_ADC_CS_PIN             GPIO_NUM_8

#define SMARTFIT_ADC_INT_PIN            GPIO_NUM_14

#define SMARTFIT_LED_1                  GPIO_NUM_5
#define SMARTFIT_LED_2                  GPIO_NUM_6
#define SMARTFIT_LED_3                  GPIO_NUM_7
#define SMARTFIT_USER_BUTTON            GPIO_NUM_16

#define WS2812_PIN 4
#define NUM_LED 8


#define SPIFLASH_PATH		            "/spiflash"
#define INTERNAL_PATH		            "/_internal" //will not shown in Webserver

//AzureIot defines
/**
 * @brief Size of the network buffer for MQTT packets.
 */
#define AzureIotNETWORK_BUFFER_SIZE    5120

/**
 * @brief The maximum number of retries for network operation with server.
 */
#define AzureIotRETRY_MAX_ATTEMPTS                      BACKOFF_ALGORITHM_RETRY_FOREVER

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server. (how long to wait max before next retry ceiling)
 */
#define AzureIotRETRY_MAX_BACKOFF_DELAY_MS              ( 30000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts. (time in 0 to this value will be used to wait before next attempt)
 */
#define AzureIotRETRY_BACKOFF_BASE_MS                   ( 1000U )

/**
 * @brief Timeout for receiving CONNACK packet in milliseconds. (answer to the connection message)
 */
#define AzureIotCONNACK_RECV_TIMEOUT_MS                 ( 10 * 1000U )

/**
 * @brief Timeout for MQTT_ProcessLoop in milliseconds. (microsoft SDK build in process loop)
 */
#define AzureIotPROCESS_LOOP_TIMEOUT_MS                 ( 500U )

/**
 * @brief Transport timeout in milliseconds for transport send and receive. (TLS socket connect)
 */
#define AzureIotTRANSPORT_SEND_RECV_TIMEOUT_MS          ( 2000U )

#endif
