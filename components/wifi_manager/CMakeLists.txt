idf_component_register(
    SRCS 
        "wifi_manager.c"
        "captive_portal.c"
        "dns_server.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        esp_wifi
        esp_netif
        esp_event
        esp_http_server
        nvs_flash
        lwip
        esp_system
        log
        smartlove_config
)
