idf_component_register(SRCS "HttpClient.cpp"
                            "HttpServer.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES    utility
                                   esp_http_client
                                   esp_http_server
                                   networking
                                   websocket
                                   esp_rom)