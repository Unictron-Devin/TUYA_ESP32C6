cmake_minimum_required(VERSION 3.16)

# This expects the ENV variable IDF_PATH to be set
if(NOT DEFINED ENV{IDF_PATH})
    message(FATAL_ERROR "IDF_PATH environment variable is not set. Please set IDF_PATH environment variable.")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(TUYA_ESP32C6)
