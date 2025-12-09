# Documentation
```cpp
The Classes named with CLIENT are to use the ESP32 as Client!!!
 _______                             _________
|       |                           |        |
| ESP32 |  ----- (1) Request -----> | Server |
| Client| <----- (2) Response ----  | Device |
|_______|                           |________|

The Classes named with SERVER are to use the ESP32 as Server!!!
 ________                              _________
|       |                            |        |
| ESP32 | <----- (1) Request ------  | Client |
| Server|  ----- (2) Response -----> | Device |
|_______|                            |________|
```

# ToDo
## HttpClientHandler
- handling timeouts on responses
- get Body part by part

** Note: ** If Body > 512 Bytes, the body must ridden part by part (not supported jet)

# Examples

## HttpServer

```cpp
SPIFFS::mount("data", "/spiflash");

HttpServer* pHttpServer = new HttpServer();
pHttpServer->setRootPath("/spiflash");
pHttpServer->start(80, false); //no SSH (not tested jet)
```

## HttpClient

```cpp
#include "esp_log.h"
#include "FreeRTOS.hpp"
#include "DNSlookup.hpp"
#include "WiFi.hpp"
#include "NetworkEventHandler.hpp"
#include "HttpClientHandler.hpp"
#include "HttpHeaders.hpp"

static char LOGTAG[] = "Main";

extern "C"
{
	void app_main(void);
}

void app_main(void)
{

	ESP_LOGI(LOGTAG, "Starting...");

	/**** SETUP WIFI ****/
	WiFi wifi;
	NetworkEventHandler netEvent;
	wifi.startSTA("DevRouter", "DEVEDC12345678$");

	// warte auf verbindung zur station
	while (wifi.getStaIp() == "0.0.0.0"){
		ESP_LOGD(LOGTAG, "Warte auf IP..");
		FreeRTOS::sleep(1000);
	}

	std::string host = "example.com";
	int port = 80;

	std::string ip;
	DNSlookup dns;
	HttpClientHandler *httpClient;

	if (dns.getIPaddres(host, port, &ip)){

		wifi->connectAP(wifi_SSID, wifi_PASS);

		// warte auf verbindung
		while (wifi->getStaIp() == "0.0.0.0")
			FreeRTOS::sleep(100);

		/**** SETUP HTTP ****/
		httpClient = new HttpClientHandler(ip, port); // ip of example.com

		httpClient->getRequest()->setMethod(HttpMethod::GET);
		httpClient->getRequest()->setPath("");		 //remote root tree -> there will added a '/' in front later
		httpClient->getRequest()->setVersion("1.1"); // set HTTP/1.1 -> there will added "HTTP/" in front later
		httpClient->getRequest()->addHeader(HttpHeader::HOST, host);
		httpClient->getRequest()->addHeader(HttpHeader::USER_AGENT, "esp32");
		httpClient->sendRequest();

		// warte antwort
		while (!httpClient->available())
			FreeRTOS::sleep(100);

		if (!httpClient->isError())		{
			std::string payload = httpClient->getResponse()->getBody();
			printf(payload.c_str());
		}

		for (;;){
			ESP_LOGD("APP", "my IP: %s\nGateway: %s", wifi.getStaIp().c_str(),
					 wifi.getApGateway().c_str());
			FreeRTOS::sleep(2000);
		}
	}
	else
		ESP_LOGE(LOGTAG, "DNS Lookup failed");

	ESP_LOGI(LOGTAG, "Exiting...");

	FreeRTOS::deleteTask();
}
```