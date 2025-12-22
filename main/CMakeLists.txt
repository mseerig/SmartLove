idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES smartlove_utils wifi_manager mqtt_client led_controller button_handler
)
