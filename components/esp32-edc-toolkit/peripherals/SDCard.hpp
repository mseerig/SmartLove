/*
 * SDCard.h
 *
 *  Created on: 09.10.2017
 *      Author: marcel.seerig
 */

#ifndef MAIN_SDCARD_H_
#define MAIN_SDCARD_H_

#include <stdio.h>
#include <string>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "sdkconfig.h"

typedef enum {
	SDIO_MODE = 0, SPI_MODE = 1, MAX_MODE,
} SD_mode_t;

/**
 * @brief This in an experimental SD card class.
 */
class SDCard {
public:
	SDCard(int max_files = 5, bool format_if_mount_failed = false);
	~SDCard();

	void initSDIO(uint32_t flags, int max_freq_khz, gpio_num_t cd = SDMMC_SLOT_NO_CD,
			gpio_num_t wp = SDMMC_SLOT_NO_WP, uint8_t width = 0);

	void initSPI(int max_freq_khz, gpio_num_t miso, gpio_num_t mosi,
			gpio_num_t sck, gpio_num_t cs, gpio_num_t cd = SDMMC_SLOT_NO_CD,
			gpio_num_t wp = SDMMC_SLOT_NO_WP);

	esp_err_t mount(std::string rootPath = "/sdcard");
	esp_err_t unmount(void);
	void printInfo(void);

	sdmmc_card_t* getCardInfo(void) {
		return card;
	}

private:
	SD_mode_t m_mode;
	sdmmc_host_t host;
	sdmmc_slot_config_t sdio_slot_config;
	sdspi_slot_config_t spi_slot_config;
	esp_vfs_fat_sdmmc_mount_config_t mount_config;
	sdmmc_card_t* card;
};

#endif /* MAIN_SDCARD_H_ */
