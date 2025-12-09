/*
 * NetworkEventHandler.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban, marcel.seerig
 *
 * A NetworkEventHandler defines a class that has methods that will be called back when a WiFi event is
 * detected.
 *
 * Typically this class is subclassed to provide implementations for the callbacks we want to handle.
 *
 * class MyHandler: public NetworkEventHandler {
 *   esp_err_t ethStart() {
 *      ESP_LOGD(tag, "MyHandler(Class): ethStart");
 *      return ESP_OK;
 *   }
 * }
 *
 * Ethernet eth(config);
 * MyHandler *eventHandler = new MyHandler();
 * eth.begin();
 *
 * The overridable functions are:
 * * esp_err_t ethConnected();
 * * esp_err_t ethDisconnected();
 * * esp_err_t ethGotIp(system_event_sta_got_ip_t event_eth_got_ip);
 * * esp_err_t ethStart();
 * * esp_err_t ethStop();
 * * esp_err_t gotIP6(system_event_got_ip6_t info);
 */

#ifndef MAIN_NetworkEventHandler_H_
#define MAIN_NetworkEventHandler_H_
#include <esp_event.h>
#include <esp_err.h>
#include <esp_log.h>

/**
 * @brief %Ethernet state event handler.
 *
 * Here is an example class that implements all the virtual functions that can be called
 * for events:
 *
 * @code{.cpp}
 * MyHandler::MyHandler() {
 * }
 *
 * MyHandler::~MyHandler() {
 * }
 *
 * esp_err_t MyHandler::ethConnected() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::ethDisconnected() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::ethGotIp(system_event_sta_got_ip_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::ethStop() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::ethStart() {
 *   return ESP_OK;
 * }
 * @endcode
 */


class NetworkEventHandler {
public:
	NetworkEventHandler(void *data=0);
	virtual ~NetworkEventHandler();

	virtual esp_err_t ethStart();
	virtual esp_err_t ethStop();
	virtual esp_err_t ethConnected();
	virtual esp_err_t ethDisconnected();

	virtual esp_err_t wifiReady();
	virtual esp_err_t staScanDone();
	virtual esp_err_t staStart();
	virtual esp_err_t staStop();
	virtual esp_err_t staConnected();
	virtual esp_err_t staDisconnected();
	virtual esp_err_t staAuthChange();

	virtual esp_err_t staWpsSuccess();
	virtual esp_err_t staWpsFailed();
	virtual esp_err_t staWpsTimeout();
	virtual esp_err_t staWpsPin();

	virtual esp_err_t apStart();
	virtual esp_err_t apStop();
	virtual esp_err_t apStaConnected();
	virtual esp_err_t apStaDisconnected();

	virtual esp_err_t apProbeRequestReceived();

	virtual esp_err_t staGotIp();
	virtual esp_err_t staLostIp();
	virtual esp_err_t apStaIpAssigned();
	virtual esp_err_t ethGotIp(esp_netif_ip_info_t * ip_info);
	virtual esp_err_t ethLostIp();
	virtual esp_err_t gotIP6();

private:
	static void ethernetEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
	static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
	static void ipEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
	void* m_data;
};

#endif /* MAIN_NetworkEventHandler_H_ */
