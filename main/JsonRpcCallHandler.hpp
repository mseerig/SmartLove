/**
 * @brief
 *
 * @file JsonRpcCallHandler.hpp
 * @author your name
 * @date 2018-08-08
 */

#ifndef JSON_RPC_CALL_HANDLER_H_
#define JSON_RPC_CALL_HANDLER_H_

#include "RtcController.hpp"
#include "ConfigurationManager.hpp"
#include "CloudController.hpp"
#include "NetworkController.hpp"
#include "JSON_RPC.hpp"
#include "ExtensionController.hpp"
#include "Authenticator.hpp"
#include "EventLog.hpp"

class CloudController;
class NetworkController;
class ExtensionController;
class Authenticator;
class ConfigurationManager;
class RtcController;

class JsonRpcCallHandler {
public:
	JsonRpcCallHandler(CloudController& cloudController, NetworkController& networkController, ExtensionController& extensionController, Authenticator& authenticator, ConfigurationManager &configurationManager, RtcController& rtcController, EventLog& eventLog);
	~JsonRpcCallHandler();

	std::string parse(std::string request);

	int authenticator_login(JsonVariant& input, JsonObject& output, void* data);
	int authenticator_changeUsernameAndPassword(JsonVariant& input, JsonObject& output, void* data);
	int authenticator_changePassword(JsonVariant& input, JsonObject& output, void* data);
	int authenticator_changeUsername(JsonVariant& input, JsonObject& output, void* data);
	int authenticator_logout(JsonVariant& input, JsonObject& output, void* data);

	int system_getInfo(JsonVariant& input, JsonObject& output, void* data);
	int system_setInfo(JsonVariant& input, JsonObject& output, void* data);
	int system_setTime(JsonVariant& input, JsonObject& output, void* data);
	int system_getTime(JsonVariant& input, JsonObject& output, void* data);
	int system_setApConfig(JsonVariant& input, JsonObject& output, void* data);
	int system_getApConfig(JsonVariant& input, JsonObject& output, void* data);
	int system_reboot(JsonVariant& input, JsonObject& output, void* data);
	int system_factoryReset(JsonVariant& input, JsonObject& output, void* data);
	int system_getHeapSize(JsonVariant& input, JsonObject& output, void* data);

	int event_getState(JsonVariant& input, JsonObject& output, void* data);
	int event_resetState(JsonVariant& input, JsonObject& output, void* data);
	int event_clear(JsonVariant& input, JsonObject& output, void* data);

	int firmware_update(JsonVariant& input, JsonObject& output, void* data);

	int ethernet_getConfig(JsonVariant& input, JsonObject& output, void* data);
	int ethernet_setConfig(JsonVariant& input, JsonObject& output, void* data);
	int ethernet_getStatus(JsonVariant& input, JsonObject& output, void* data);

	int wifi_getConfig(JsonVariant& input, JsonObject& output, void* data);
	int wifi_setConfig(JsonVariant& input, JsonObject& output, void* data);
	int wifi_getStatus(JsonVariant& input, JsonObject& output, void* data);

	int cloud_setConfig(JsonVariant& input, JsonObject& output, void* data);
	int cloud_getConfig(JsonVariant& input, JsonObject& output, void* data);

	int extension_get(JsonVariant& input, JsonObject& output, void* data);
	int extension_set(JsonVariant& input, JsonObject& output, void* data);

private:
	CloudController&         	m_cloudController;
	NetworkController&			m_networkController;
	ExtensionController&		m_extensionController;
	Authenticator&				m_authenticator;
	ConfigurationManager&		m_configurationManager;
	RtcController&				m_rtcController;
	EventLog&					m_eventLog;

	JSON_RPC 					m_rpc;
};

#endif