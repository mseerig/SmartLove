# Usage
First of all, you shold create an Ethernet/WiFi instance.
```cpp
#include "WiFi.hpp"
WiFi wifi;
```

Very impotent is now the ```NetworkEventHandler``` instance. These will init the tcpip adapter and will handle the incoming events.

```cpp
#include "WiFi.hpp"
WiFi wifi;
NetworkEventHandler networkHandler;
```

Now, all settings are done and you can use the WiFi class:
```cpp
wifi.startSTA("DevRouter", "DEVEDC12345678$");
// warte auf verbindung zur station
while (wifi.getStaIp() == "0.0.0.0"){
    ESP_LOGD(LOGTAG, "Warte auf IP..");
    FreeRTOS::sleep(1000);
}
```

If you want to have an explicit eventHandler for Ethernet, or Wifi, you can now ```networkHandler.setWifiEventHandler(myWifiHandler);```.

**Note:** The ```myWifiHandler``` is an instance of an class which overrides the WifiEventHandler-Class.

Example:
```cpp
class MyWifiHandler: public WifiEventHandler{
    public:
        esp_err_t staGotIp(system_event_sta_got_ip_t info) {
            ESP_LOGD(LOG_TAG, "myHandler staGotIp");
            return ESP_OK;
        }
};
```
And now set the Handler:
```cpp
MyWifiHandler*	myWifiHandler;
networkHandler.setWifiEventHandler(myWifiHandler);
};
```