# Example

- Connect via wifi to Websocket Server
- Print received messages to console
- Sending "Hallo Welt!!" every two seconds

```cpp
/*
 * testWebsocketClient.cpp
 *
 *  Created on: 27.10.2017
 *      Author: marcel.seerig
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <esp_log.h>

#include "HttpClientHandler.h"
#include "Task.h"
#include "FreeRTOS.h"

#include "WiFi.h"

#include "sdkconfig.h"

using namespace std;

extern "C" {
void app_main(void);
}

static WiFi *wifi;

HttpClientHandler *httpClient;
WebSocket* m_websocket;

std::string wifi_SSID 			= "DEVEDC";
std::string wifi_PASS 			= "12345678";

std::string websocket_SERVERIP 	= "192.168.1.6";
int websocket_PORT 				= 5555;

class WebSocketEventHandler: public WebSocketHandler {
	void onMessage(WebSocketInputStreambuf *pWebSocketInputStreambuf) override {
		ESP_LOGD("WebSocketEventHandler", "Message arrived!");
		std::stringstream buffer;
		buffer << pWebSocketInputStreambuf;
		std::string msg = buffer.str();

		if (msg != "") {
			ESP_LOGD("WebSocketEventHandler", "Message: %s", msg.c_str());
		}
	}
	void onError() {
		ESP_LOGD("WebSocketEventHandler", "onError");
	}
	void onClose() {
		ESP_LOGD("WebSocketEventHandler", "onClose");
	}
};
static WebSocketEventHandler* webSocketHandler;

class Main_Task: public Task {
	bool createWebsocket() {

		ESP_LOGD("APP", "Try to create a new HttpClientHandler");
		httpClient = new HttpClientHandler((char*) websocket_SERVERIP.c_str(), websocket_PORT);

		if (!httpClient->createWebsocket()) {
			ESP_LOGE("APP", "Can not create websocket!");
			return 0;
		}

		ESP_LOGD("APP", "Waiting for Websocket...");
		while (!httpClient->isWebsocket()) {
			if(httpClient->isError()) return 0;
		}

		m_websocket = httpClient->getWebsocket();
		webSocketHandler = new WebSocketEventHandler;
		m_websocket->setHandler(webSocketHandler);
		m_websocket->startReader();

		return 1;
	}

	void run(void *args) override {

		wifi = new WiFi;

		wifi->connectAP(wifi_SSID, wifi_PASS);

		// warte auf verbindung
		while (wifi->getStaIp() == "")
			FreeRTOS::sleep(1000);

		//warte auf websocket
		while(!createWebsocket())
			FreeRTOS::sleep(1000);

		for (;;) {
			while (wifi->getStaIp() == "")
				FreeRTOS::sleep(1000);
			/********************** RUN APP **********************/
			/*
			 * At this point, i'am connected to my host and the
			 * Websocket cliet is running. Now, I can do my
			 * Application stuff. This Routine is interrupted by
			 * disconnecting the host from my device.
			 */

			//Socket died?
			if(!m_websocket->getSocket().isValid()){
				ESP_LOGD("APP", "my Socket is dead!");
				//Delete HttpClientHandler or websocket if it was created
				if (httpClient) delete (httpClient);
				m_websocket = nullptr; // websocket pointer is now invalid!

				// Try to create a new HttpClientHandler
				while(!createWebsocket())
					FreeRTOS::sleep(1000);

			}

			m_websocket->send("Hallo Welt!!", WebSocket::SEND_TYPE_TEXT);

			ESP_LOGD("APP", "my IP: %s\nGateway: %s", wifi->getStaIp().c_str(),
					wifi->getApGateway().c_str());
			FreeRTOS::sleep(2000);
		}
	}
}
;

static Main_Task *main_Task;

void app_main(void) {

	main_Task = new Main_Task();
	main_Task->start();

}
```