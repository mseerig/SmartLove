cmake_minimum_required(VERSION 3.16)

# ESP32 Target setzen VOR project.cmake
set(IDF_TARGET esp32)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(smartlove)
