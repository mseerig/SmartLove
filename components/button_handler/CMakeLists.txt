idf_component_register(
    SRCS "button_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES driver mqtt_client smartlove_utils smartlove_config
)
