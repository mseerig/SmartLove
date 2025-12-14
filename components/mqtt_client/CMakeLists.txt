idf_component_register(
    SRCS "smartlove_mqtt.c"
    INCLUDE_DIRS "include"
    REQUIRES mqtt esp_event esp_netif nvs_flash log
)
