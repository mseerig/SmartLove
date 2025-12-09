/*
 * SPIFFS.cpp
 *
 *  Created on: 08.10.2018
 *      Author: marcel.seerig
 */

#include "SPIFFS.hpp"

#include <esp_log.h>
#include <esp_spiffs.h>

static char LOGTAG[] = "SPIFFS";

/**
 * @brief SPIFFS mount
 * @param montPath set the mount path.
 */
esp_err_t SPIFFS::mount(std::string label, std::string mountPath){
	esp_vfs_spiffs_conf_t conf = esp_vfs_spiffs_conf_t();
	conf.base_path = mountPath.c_str();
	conf.partition_label = label.c_str();
	conf.max_files = 5;
	conf.format_if_mount_failed = false;


	esp_err_t ret = ::esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(LOGTAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(LOGTAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(LOGTAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}else ESP_LOGD(LOGTAG, "Mounted '%s' as '%s'!", label.c_str(), mountPath.c_str());
	return ret;
}