idf_component_register(SRCS "Azure.cpp"
                            "MQTT.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES    networking
                                   mqtt
                                   sntp
                       EMBED_TXTFILES  "digicert_baltimore_root.pem"         
                                   )