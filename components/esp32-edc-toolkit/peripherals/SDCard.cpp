/*
 * SDCard.cpp
 *
 *  Created on: 09.10.2017
 *      Author: marcel.seerig
 */

#include "SDCard.hpp"

#include "sdkconfig.h"

static const char *TAG = "SDCard";

/**
 * @brief SD card class constructor.
 * @param [in] Set the maximum number of open files.
 * @param [in] Set format if mount failed (true/false).
 */
SDCard::SDCard(int max_files, bool format_if_mount_failed) {
	ESP_LOGI(TAG, "Initializing SD card");
	card = nullptr;
	m_mode = MAX_MODE;

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	mount_config.format_if_mount_failed = format_if_mount_failed;
	mount_config.max_files = max_files;
} // SDCard

/**
 * @brief Configures the SDIO host and slot2.
 * @param [in] flags defining host properties.
 * @param [in] max frequency supported by the host.
 * @param [in] GPIO number of card detect signal.
 * @param [in] GPIO number of write protect signal.
 * @param [in] Bus width used by the slot.
 * @return N/A.
 */
void SDCard::initSDIO(uint32_t flags, int max_freq_khz, gpio_num_t cd,
		gpio_num_t wp, uint8_t width) {

	ESP_LOGI(TAG, "Using SDMMC peripheral");
	m_mode = SDIO_MODE;

	host.flags = flags;
	host.slot = SDMMC_HOST_SLOT_1;
	host.max_freq_khz = max_freq_khz;
	host.io_voltage = 3.3f;
	host.init = &sdmmc_host_init;
	host.set_bus_width = &sdmmc_host_set_bus_width;
	host.get_bus_width = &sdmmc_host_get_slot_width;
	host.set_card_clk = &sdmmc_host_set_card_clk;
	host.do_transaction = &sdmmc_host_do_transaction;
	host.deinit = &sdmmc_host_deinit;
	host.command_timeout_ms = 0;

	//load config from menuconfig settings
	// This initializes the slot card detect (CD) and write protect (WP) signals.
	// Modify menuconfig and insert a pin number instad of "-1" if your board has these signals.

	sdmmc_slot_config_t m_sdio_slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	sdio_slot_config = m_sdio_slot_config;
	sdio_slot_config.gpio_cd = cd;
	sdio_slot_config.gpio_wp = wp;
	sdio_slot_config.width = width;
} // initSDIO

/**
 * @brief Configures the SPI host and slot.
 * @param [in] max frequency supported by the host.
 * @param [in] GPIO number of MISO signal.
 * @param [in] GPIO number of MOSI signal.
 * @param [in] GPIO number of clock signal.
 * @param [in] GPIO number of chip select signal.
 * @param [in] GPIO number of card detect signal.
 * @param [in] GPIO number of write protect signal.
 * @return N/A.
 */
void SDCard::initSPI(int max_freq_khz, gpio_num_t miso, gpio_num_t mosi,
		gpio_num_t sck, gpio_num_t cs, gpio_num_t cd, gpio_num_t wp) {

	ESP_LOGI(TAG, "Using SPI peripheral");
	m_mode = SPI_MODE;

	host.flags = SDMMC_HOST_FLAG_SPI;
	host.slot = SPI2_HOST;
	host.max_freq_khz = max_freq_khz;
	host.io_voltage = 3.3f;
	host.init = &sdspi_host_init;
	host.set_bus_width = NULL;
	host.set_card_clk = &sdspi_host_set_card_clk;
	host.do_transaction = &sdspi_host_do_transaction;
	host.deinit = &sdspi_host_deinit;
	host.command_timeout_ms = 0;

	//load config from menuconfig settings
	sdspi_slot_config_t m_spi_slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
	spi_slot_config = m_spi_slot_config;
	spi_slot_config.gpio_miso = miso;
	spi_slot_config.gpio_mosi = mosi;
	spi_slot_config.gpio_sck = sck;
	spi_slot_config.gpio_cs = cs;
	// This initializes the slot card detect (CD) and write protect (WP) signals.
	// Modify menuconfig and insert a pin number instad of "-1" if your board has these signals.
	spi_slot_config.gpio_cd = cd;
	spi_slot_config.gpio_wp = wp;
} // initSPI

/**
 * @brief Mount the SD card.
 * Note: This an all-in-one convenience function.
 * Please check its source code and implement error recovery when developing
 * production applications.
 * @param [in] path were the chard is mounted.
 * @return error flags.
 */
esp_err_t SDCard::mount(std::string rootPath) {

	esp_err_t ret = ESP_FAIL;
	if (m_mode == SDIO_MODE) {
		ret = ::esp_vfs_fat_sdmmc_mount(rootPath.c_str(), &host,
				&sdio_slot_config, &mount_config, &card);
	} else if (m_mode == SPI_MODE) {
		ret = ::esp_vfs_fat_sdmmc_mount(rootPath.c_str(), &host, &spi_slot_config,
				&mount_config, &card);
	} else {
		ESP_LOGE(TAG, "Init SD Card, before use!!!");
	}
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem.");

		} else {
			ESP_LOGE(TAG,
					"Failed to initialize the card (%d). " "Make sure SD card lines have pull-up resistors in place.",
					ret);
		}
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "Mount with success!");
	return ESP_OK;
} // mount

/**
 * @brief Print SD card informations.
 */
void SDCard::printInfo(void) {
	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
} // printInfo

/**
 * @brief Unmount the SD card.
 * @param N/A.
 * @return error flags.
 */
esp_err_t SDCard::unmount(void) {
	// All done, unmount partition and disable SDMMC or SPI peripheral
	ESP_LOGI(TAG, "unmount!");
	return esp_vfs_fat_sdmmc_unmount();
} // unmount

/**
 * @brief destructor of the SDCard class.
 */
SDCard::~SDCard(void) {
	ESP_LOGI(TAG, "~SDCard()");
	this->unmount();
} // ~SDCard
