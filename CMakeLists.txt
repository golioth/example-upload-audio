cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS submodules/golioth-firmware-sdk/port/esp_idf/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(golioth_audio_upload)
