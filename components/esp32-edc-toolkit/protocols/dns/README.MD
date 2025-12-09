# Documentation
Supported now:
- DNSlookup (hostname to ip)
- MDNS (name of the device)

ToDo:
- mDNS of the ESP32

# Example for mDNS
```cpp
MDNS mdns("EDC", "SmartFit-Device-123");
mdns.addService("_http", "_tcp", 80);
```cpp

# Example for DNS Lookup
```cpp
std::string host = "example.com";
std::string ip;
DNSlookup dns;
if(dns.getIPaddres(host, 80, &ip)){
	ESP_LOGD(LOGTAG, "%s : %s", host.c_str(), ip.c_str());

}else
	ESP_LOGE(LOGTAG, "DNS Lookup failed");
```cpp
