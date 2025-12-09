idf_component_register(SRCS "FreeRTOS.cpp"
                            "FreeRTOSTimer.cpp"
                            "Task.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES    esp_ringbuf
                                   freertos)