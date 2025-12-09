idf_component_register(SRCS "DNS.cpp"
							"NetworkEventHandler.cpp"
							"Ethernet.cpp"
                            "Socket.cpp"
                            "SockServ.cpp"
                            "SSLUtils.cpp"
                            "WiFi.cpp"

                       INCLUDE_DIRS "."
                       REQUIRES    utility
                                   freertoscpp
                                   mdns
							lwip)