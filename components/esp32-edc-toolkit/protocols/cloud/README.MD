# Example

- Connect via wifi to broker
- sending in topic "test" periodically

**Attention:** The PubSubClient-Class will replaced in V0.2 with the MQTT-Class!

Example with MQTT-Class:

```cpp
#include "esp_log.h"
#include "FreeRTOS.hpp"
#include "WiFi.hpp"
#include "NetworkEventHandler.hpp"
#include "MQTT.hpp"

#include "HttpHeaders.hpp"

static char LOGTAG[] = "Main";

extern "C"{
	void app_main(void);
}

class MyMqttHandler: public MQTTeventHandler{
	public:
		MyMqttHandler(void* data = nullptr){
			m_data = data;
			m_connected = false;
		}
		void onConnected(esp_mqtt_event_handle_t event) override{
			m_connected = true;
		}
		void onDisconnected(esp_mqtt_event_handle_t event) override{
			m_connected = false;
		}
		void onData(esp_mqtt_event_handle_t event) override{
			std::string topic = event->topic;
			topic = topic.substr(0,event->topic_len);
			std::string payload = event->data;
			payload = payload.substr(0,event->data_len);
			ESP_LOGD(LOGTAG, "GOT FORM '%s' '%s'", topic.c_str(), payload.c_str());
		}
		bool isConnected(){
			return m_connected;
		}
	private:
		bool m_connected;
		void* m_data;
};

void app_main(void){
	ESP_LOGI(LOGTAG, "---------------------------------------------");
	ESP_LOGI(LOGTAG, "Starting...");

	WiFi wifi;
	NetworkEventHandler netEvent;
	wifi.startSTA("DevRouter", "DEVEDC12345678$");

	// warte auf verbindung zur station
	while (wifi.getStaIp() == "0.0.0.0"){
		ESP_LOGD(LOGTAG, "Warte auf IP..");
		FreeRTOS::sleep(1000);
	}

	MQTT mqtt(MQTT_TRANSPORT_OVER_TCP, "10.0.4.35", 1883);
	MyMqttHandler* mqttHandler = new MyMqttHandler;
	mqtt.setEventHandler(mqttHandler);

	mqtt.setClientId("ESP32_dev1");
	// mqtt.setAuthorization("user", "pass");
	mqtt.setLastWill("lwt", "Offline",0,true);

	mqtt.start();

	while(!mqttHandler->isConnected())
		FreeRTOS::sleep(1000);

	mqtt.publish("lwt","Online",0,true);

	mqtt.subscribe("input", 1);

	for(;;) {
		ESP_LOGI(LOGTAG, "Send...");
		mqtt.publish("test","test",1);
		FreeRTOS::sleep(2000);
	}
	FreeRTOS::deleteTask();
}
```