idf_component_register(SRCS "ADC.cpp"
                            "DeepSleep.cpp"
                            "GPIO.cpp"
                            "I2C.cpp"
                            "I2CArbiter.cpp"
                            "NVS.cpp"
                            "SDCard.cpp"
                            "SPIFFS.cpp"
                            "System.cpp"
                            "UART.cpp"
                            "PWM.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES    nvs_flash
                                   esp_adc_cal
                                   utility
                                   spiffs
                                   driver
                                   main)