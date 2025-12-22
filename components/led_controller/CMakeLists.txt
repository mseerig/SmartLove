idf_component_register(
    SRCS "led_controller.c" "ws2812_rmt.c"
    INCLUDE_DIRS "include"
    REQUIRES driver json smartlove_config
)
