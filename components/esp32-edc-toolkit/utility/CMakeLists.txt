idf_component_register(SRCS "SHA1.cpp"
			             "AES.cpp"
                            "FATFS_VFS.cpp"
                            "File.cpp"
                            "FileSystem.cpp"
                            "GeneralUtils.cpp"
                            "HMAC.cpp"
                            "itoa.c"
                            "JSON_RPC.cpp"
                            "Memory.cpp"
                            "Print.cpp"
                            "SHA256.cpp"
                            "SysTick.cpp"
                            "WString.cpp"
                       INCLUDE_DIRS "." "./json/."
                       REQUIRES    fatfs
                                   mbedtls
                                   app_update
                                   http
                                   freertoscpp
                                   peripherals)