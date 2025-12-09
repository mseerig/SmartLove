/**
 * @brief
 *
 * @file JsonRpcCallHandler.cpp
 * @author your name
 * @date 2018-08-08
 */

#include "JsonRpcCallHandler.hpp"
#include "CallbackWrapper.hpp"
#include "GeneralUtils.hpp"
#include "NVS.hpp"
#include "ArduinoJson.h"
#include "OtaController.hpp"
#include "System.hpp"
#include <functional>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"

static char TAG[] = "JsonRpcCallHandler";


JsonRpcCallHandler::JsonRpcCallHandler(CloudController& cloudController, NetworkController& networkController, ExtensionController& extensionController, Authenticator& authenticator, ConfigurationManager &configurationManager, RtcController& rtcController, EventLog& eventLog)
: m_cloudController(cloudController),
  m_networkController(networkController),
  m_extensionController(extensionController),
  m_authenticator(authenticator),
  m_configurationManager(configurationManager),
  m_rtcController(rtcController),
  m_eventLog(eventLog){

 	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::authenticator_login, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)), "authenticator.login");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::authenticator_changeUsernameAndPassword, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3)), "authenticator.changeUsernameAndPassword");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::authenticator_changeUsername, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3)), "authenticator.changeUsername");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::authenticator_changePassword, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3)), "authenticator.changePassword");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::authenticator_logout, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "authenticator.logout");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_getInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.getInfo");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_setInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.setInfo");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_getTime, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.getTime");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_setTime, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.setTime");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_getApConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.getApConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_setApConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.setApConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::firmware_update, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.firmwareUpdate");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_reboot, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.reboot");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_factoryReset, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.factoryReset");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::system_getHeapSize, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "system.getHeapSize");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::event_getState, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "event.getState");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::event_resetState, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "event.resetState");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::event_clear, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "event.clear");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::ethernet_getConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "ethernet.getConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::ethernet_setConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "ethernet.setConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::ethernet_getStatus, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "ethernet.getStatus");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::wifi_setConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "wifi.setConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::wifi_getConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "wifi.getConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::wifi_getStatus, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "wifi.getStatus");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::cloud_getConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "cloud.getConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::cloud_setConfig, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "cloud.setConfig");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::extension_get, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "extension.get");

	m_rpc.addMethod(GETCB(callback_function, JsonRpcCallHandler)(std::bind(&JsonRpcCallHandler::extension_set, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), "extension.set");
}

JsonRpcCallHandler::~JsonRpcCallHandler() {

}

std::string JsonRpcCallHandler::parse(std::string request) {
	return m_rpc.parse(request);
}

int JsonRpcCallHandler::authenticator_login(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant username = input["username"];
	JsonVariant password = input["password"];

	if(username.isNull() || password.isNull()) return JSONRPC_INVALID_PARAMETER;

	ESP_LOGD(TAG, "Username '%s' Password '%s'", username.as<char*>(), password.as<char*>());

	//check username and password
	std::string token = "";

	auth_state_t state = m_authenticator.login(username.as<std::string>(), password.as<std::string>(), &token);
	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	if(state == AUTH_STATE_OK){
		result["apiToken"] = token;
		result["authLevel"] = (int)m_authenticator.getUser(username.as<std::string>())->getAuthLevel();
	}
	return 0;
}

int JsonRpcCallHandler::authenticator_changeUsernameAndPassword(JsonVariant& input, JsonObject& output, void* data){

	JsonVariant newUsername = input["newUsername"];
	JsonVariant newPassword = input["newPassword"];
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull() || newUsername.isNull() || newPassword.isNull())
		return JSONRPC_INVALID_PARAMETER;

	//check username and password
	auth_state_t state = m_authenticator.check(username.as<std::string>(), apiToken.as<std::string>(), AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		state = m_authenticator.changeUsernameAndPassword(username.as<std::string>(), newUsername.as<std::string>(), newPassword.as<std::string>());
	}
	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::authenticator_changeUsername(JsonVariant& input, JsonObject& output, void* data){

	JsonVariant newUsername = input["newUsername"];
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull() || newUsername.isNull())
		return JSONRPC_INVALID_PARAMETER;

	//check username and password
	auth_state_t state = m_authenticator.check(username.as<std::string>(), apiToken.as<std::string>(), AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK) state = m_authenticator.changeUsername(username.as<std::string>(), newUsername.as<std::string>());

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::authenticator_changePassword(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant newPassword = input["newPassword"];
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull() || newPassword.isNull())
		return JSONRPC_INVALID_PARAMETER;

	//check username and password
	auth_state_t state = m_authenticator.check(username.as<std::string>(), apiToken.as<std::string>(), AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK)	state = m_authenticator.changePassword(username.as<std::string>(), newPassword.as<std::string>());

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::authenticator_logout(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	//check username and password
	auth_state_t state = m_authenticator.check(username.as<std::string>(), apiToken.as<std::string>(), AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK) state = m_authenticator.logout(username.as<std::string>());

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_reboot(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		System::restart();
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_factoryReset(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		m_extensionController.factoryReset();
		m_configurationManager.factoryReset();
		System::restart();
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_getHeapSize(JsonVariant& input, JsonObject& output, void* data){
	JsonObject result = output.createNestedObject("result");
	result["heap"] = System::getFreeHeapSize();
	return 0;
}

int JsonRpcCallHandler::system_getInfo(JsonVariant& input, JsonObject& output, void* data){
	// without input parameters...
	JsonObject result = output.createNestedObject("result");
	result.set(m_configurationManager.getSystemInfo());
	return 0;
}

int JsonRpcCallHandler::system_setInfo(JsonVariant& input, JsonObject& output, void* data){
	SystemConfiguration_t conf = m_configurationManager.getSystemConfiguration();

	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		JsonVariant name = input["name"];
		JsonVariant location = input["location"];


		if(name.isNull() && location.isNull() )
			return JSONRPC_INVALID_PARAMETER;

		if(!name.isNull()) {
			conf.name = name.as<std::string>();
		}
		if(!location.isNull()) {
			conf.location = location.as<std::string>();
		}

		m_configurationManager.setSystemConfiguration(conf);
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_getTime(JsonVariant& input, JsonObject& output, void* data){

	TimeConfiguration_t conf = m_configurationManager.getTimeConfiguration();
	JsonObject result = output.createNestedObject("result");
	result["enableSNTP"] = conf.enableSNTP;
	result["sntpServer"] = conf.sntpServer;
	result["enableTimeOutput"] = conf.enableTimeOutput;
	result["timezone"] = conf.timezone;
	result["location"] = conf.location;
	result["timestamp"] = System::getLocalTimestamp(conf.timezone);
	result["utcTime"] = (uint32_t) System::getTime();
	result["upTime"] = System::getUptime();
	result["rtcWorking"] = m_rtcController.isRtcWorking();
	return 0;
}

int JsonRpcCallHandler::system_setTime(JsonVariant& input, JsonObject& output, void* data){
	TimeConfiguration_t conf = m_configurationManager.getTimeConfiguration();

	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		JsonVariant enableSNTP = input["enableSNTP"];
		JsonVariant enableTimeOutput = input["enableTimeOutput"];
		JsonVariant timezone = input["timezone"];
		JsonVariant location = input["location"];
		JsonVariant manualTime = input["manualTime"];
		JsonVariant sntpServer = input["sntpServer"];

		if(enableSNTP.isNull() && manualTime.isNull() )
			return JSONRPC_INVALID_PARAMETER;

		if(!sntpServer.isNull()) {
			if(!GeneralUtils::isIp(conf.sntpServer) && !GeneralUtils::isHostname(conf.sntpServer)){
				return JSONRPC_INVALID_PARAMETER;
			}
			conf.sntpServer = sntpServer.as<std::string>();
		}

		if(!enableSNTP.isNull()) conf.enableSNTP = enableSNTP.as<bool>();
		if(!manualTime.isNull()) conf.enableSNTP = false;

		if(!timezone.isNull()) conf.timezone = timezone.as<std::string>();

		if(!location.isNull()) conf.location = location.as<std::string>();

		if(!enableTimeOutput.isNull()) conf.enableTimeOutput = enableTimeOutput.as<bool>();

		if(!manualTime.isNull()){
			time_t newTime = (uint32_t)manualTime.as<unsigned long>();
			System::setLocalTime(newTime, conf.timezone);
			m_rtcController.getRtc()->updateRtcTimeFromSystem();
		}

		m_configurationManager.setTimeConfiguration(conf);
		m_networkController.setSntpConfigurationChanged();

	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_setApConfig(JsonVariant& input, JsonObject& output, void* data){
	ApConfiguration_t conf = m_networkController.getApConfiguration();

	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		JsonVariant alwaysActive = input["alwaysActive"];
		JsonVariant ssid = input["ssid"];
		JsonVariant password = input["password"];

		if(alwaysActive.isNull())
			return JSONRPC_INVALID_PARAMETER;

		conf.alwaysActive = alwaysActive.as<bool>();

		if(!ssid.isNull()) conf.ssid = ssid.as<std::string>();

		if(!password.isNull()) conf.password = password.as<std::string>();

		if(conf.ssid == "" || conf.password == "" || (conf.password.length() < 8))
			return JSONRPC_INVALID_PARAMETER;

		m_networkController.setApConfiguration(conf);
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::system_getApConfig(JsonVariant& input, JsonObject& output, void* data){
	// without input parameters...
	ApConfiguration_t conf = m_networkController.getApConfiguration();
	JsonObject result = output.createNestedObject("result");

	result["alwaysActive"] = conf.alwaysActive;
	result["ssid"] = conf.ssid;
	return 0;
}

int JsonRpcCallHandler::event_getState(JsonVariant& input, JsonObject& output, void* data){
	// without input parameters...
	JsonObject result = output.createNestedObject("result");

	result["code"] = (int) m_eventLog.getStatus();
	return 0;
}

int JsonRpcCallHandler::event_resetState(JsonVariant& input, JsonObject& output, void* data){

	JsonObject result = output.createNestedObject("result");
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state != AUTH_STATE_OK){
		result["info"] = m_authenticator.stateToString(state);
		result["state"] = (int)state;
		result["ret"] = -1;
		return 0;
	}

	// reset logStatus
	esp_err_t ret = m_eventLog.resetStatus();
	result["ret"] = (int) ret;
	return 0;
}

int JsonRpcCallHandler::event_clear(JsonVariant& input, JsonObject& output, void* data){
	JsonObject result = output.createNestedObject("result");
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_ADMIN);
	if(state != AUTH_STATE_OK){
		result["info"] = m_authenticator.stateToString(state);
		return 0;
	}

	// clear logStatus
	esp_err_t ret = m_eventLog.clear();
	result["ret"] = (int) ret;
	return 0;
}

int JsonRpcCallHandler::firmware_update(JsonVariant& input, JsonObject& output, void* data){
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];
	JsonVariant url = input["url"];

	if(username.isNull() || apiToken.isNull() || url.isNull())
		return JSONRPC_INVALID_PARAMETER;

	std::string link = url.as<std::string>();
	if(!GeneralUtils::isUri(link)) return JSONRPC_INVALID_PARAMETER;
	ESP_LOGD(TAG, "Starting OTA Update with '%s'", link.c_str());

	JsonObject result = output.createNestedObject("result");

	auth_state_t state = m_authenticator.check(username.as<std::string>(), apiToken.as<std::string>(), AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		OtaController m_otaController(link);

		while(m_otaController.isDone() != true) FreeRTOS::sleep(100);

		FreeRTOS::sleep(3000); //need time to cool down

		result["state"] = m_otaController.getResult();
		result["info"] = m_otaController.getResultString();
	}else{
		result["state"] = OTA_STATE_MAX+(int)state;
		result["info"] = m_authenticator.stateToString(state);
	}

	return 0;
}

int JsonRpcCallHandler::ethernet_getStatus(JsonVariant& input, JsonObject& output, void* data) {
	JsonObject result = output.createNestedObject("result");

	if (m_networkController.isEthernetActive()) {
		result["state"] = "connected";
		result["ip"] = m_networkController.getEthernetIP();
		result["gateway"] = m_networkController.getEthernetGateway();

	}else{
		result["state"] = "disonnected";
		result["ip"] = "";
		result["gateway"] = "";
	}

	return 0; // means success
}

int JsonRpcCallHandler::wifi_getStatus(JsonVariant& input, JsonObject& output, void* data){
	JsonObject result = output.createNestedObject("result");

	if (m_networkController.isWifiStationActive()) {
		result["state"] = "connected";
		result["ip"] = m_networkController.getWifiIP();
		result["gateway"] = m_networkController.getWifiGateway();

	}else{
		result["state"] = "disconnected";
		result["ip"] = "";
		result["gateway"] = "";
	}

	return 0; // means success
}

int JsonRpcCallHandler::ethernet_getConfig(JsonVariant& input, JsonObject& output, void* data){
	EthernetConfiguration_t conf = m_networkController.getEthernetConfiguration();

	JsonObject result = output.createNestedObject("result");

	result["active"] = conf.active;
	result["useDHCP"] = conf.useDHCP;
	if(!conf.useDHCP){
		result["ip"] = conf.ip;
		result["gateway"] = conf.gateway;
		result["netmask"] = conf.netmask;
	}
	result["user_dns"] = conf.user_dns;
	if(conf.user_dns){
		result["dns_main"] = conf.dns_main;
		result["dns_fallback"] = conf.dns_fallback;
	}
	return 0; // means success
}

int JsonRpcCallHandler::wifi_getConfig(JsonVariant& input, JsonObject& output, void* data){
	WifiConfiguration_t conf = m_networkController.getWifiConfiguration();

	JsonObject result = output.createNestedObject("result");
	result["ssid"] = conf.ssid;
	result["active"] = conf.active;
	return 0; // means success
}

int JsonRpcCallHandler::ethernet_setConfig(JsonVariant& input, JsonObject& output, void* data){
	EthernetConfiguration_t conf = m_networkController.getEthernetConfiguration();

	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){
		JsonVariant ip = input["ip"];
		JsonVariant gateway = input["gateway"];
		JsonVariant netmask = input["netmask"];
		JsonVariant useDHCP = input["useDHCP"];
		JsonVariant active = input["active"];
		JsonVariant user_dns = input["user_dns"];
		JsonVariant dns_main = input["dns_main"];
		JsonVariant dns_fallback = input["dns_fallback"];

		if(active.isNull()) return JSONRPC_INVALID_PARAMETER;
		conf.active = active.as<bool>();

		if (conf.active){
			if(useDHCP.isNull()) return JSONRPC_INVALID_PARAMETER;
			conf.useDHCP = useDHCP.as<bool>();

			if(!conf.useDHCP){
				if(ip.isNull() || gateway.isNull() || netmask.isNull())
				return JSONRPC_INVALID_PARAMETER;

				conf.ip = ip.as<std::string>();
				conf.gateway = gateway.as<std::string>();
				conf.netmask = netmask.as<std::string>();

				if(!GeneralUtils::isIp(conf.ip) || !GeneralUtils::isIp(conf.gateway) || !GeneralUtils::isNetmask(conf.netmask))
					return JSONRPC_INVALID_PARAMETER;
			}else{
				conf.ip = "";
				conf.gateway = "";
				conf.netmask = "";
			}

			conf.user_dns = user_dns.as<bool>();
			if(conf.user_dns){
				conf.dns_main = dns_main.as<std::string>();
				conf.dns_fallback = dns_fallback.as<std::string>();
			}else{
				conf.dns_main = "";
				conf.dns_fallback = "";
			}

			//disable wifi sta
			WifiConfiguration_t wifi_conf = m_networkController.getWifiConfiguration();
			if(wifi_conf.active){
				wifi_conf.active = false;
				m_networkController.setWifiConfiguration(wifi_conf);
			}
		}

		m_networkController.setEthernetConfiguration(conf);
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::wifi_setConfig(JsonVariant& input, JsonObject& output, void* data){
	WifiConfiguration_t conf = m_networkController.getWifiConfiguration();

	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){

		JsonVariant ssid = input["ssid"];
		JsonVariant password = input["password"];
		JsonVariant active = input["active"];

		if(active.isNull())	return JSONRPC_INVALID_PARAMETER;

		conf.active = active.as<bool>();

		if(conf.active){
			if(ssid.isNull() || password.isNull()) return JSONRPC_INVALID_PARAMETER;

			conf.ssid = ssid.as<std::string>();
			conf.password = password.as<std::string>();

			//disable ethernet
			EthernetConfiguration_t eth_conf = m_networkController.getEthernetConfiguration();
			if (eth_conf.active)  {
				eth_conf.active = false;
				m_networkController.setEthernetConfiguration(eth_conf);
			}
		}

		m_networkController.setWifiConfiguration(conf);
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);
	return 0;
}

int JsonRpcCallHandler::cloud_getConfig(JsonVariant& input, JsonObject& output, void* data){

	int result = m_cloudController.parseJsonrpcGet(input, output);
	return result;
}

int JsonRpcCallHandler::cloud_setConfig(JsonVariant& input, JsonObject& output, void* data){
	int ret = 0;
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){

		ret = m_cloudController.parseJsonrpcSet(input, output);
		if(ret != 0) return ret;
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);

	return ret;
}

int JsonRpcCallHandler::extension_get(JsonVariant& input, JsonObject& output, void* data){

	int result = m_extensionController.parseJsonrpcGet(input, output);
	return result;
}

int JsonRpcCallHandler::extension_set(JsonVariant& input, JsonObject& output, void* data){
	int ret = 0;
	JsonVariant username = input["auth"]["username"];
	JsonVariant apiToken = input["auth"]["apiToken"];

	if(username.isNull() || apiToken.isNull())
		return JSONRPC_INVALID_PARAMETER;

	// check username and apiToken
	auth_state_t state = m_authenticator.check(username, apiToken, AUTH_LEVEL_USER);
	if(state == AUTH_STATE_OK){

		ret = m_extensionController.parseJsonrpcSet(input, output);
		if(ret != 0) return ret;
	}

	JsonObject result = output.createNestedObject("result");
	result["state"] = (int)state;
	result["info"] = m_authenticator.stateToString(state);

	return ret;
}