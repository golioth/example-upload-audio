set(esp_idf_common "../submodules/golioth-firmware-sdk/examples/esp_idf/common")

if(CONFIG_IDF_TARGET_ESP32)
    message("################## Building for the m5stack Core2 ##########################")
    set(bsp_srcs
        "m5stack_core2/m5stack_core2.c"
        "m5stack_core2/axp192/axp192.c"
        "m5stack_core2/axp192/axp192_i2c.c"
        "m5stack_core2/i2c_bus/i2c_device.c"
    )

    set(bsp_includes
        "m5stack_core2"
        "m5stack_core2/axp192"
        "m5stack_core2/i2c_bus"
    )
endif(CONFIG_IDF_TARGET_ESP32)

if(CONFIG_IDF_TARGET_ESP32S3)
    message("################## Building for the m5stack CoreS3 ##########################")
endif(CONFIG_IDF_TARGET_ESP32S3)

idf_component_register(SRCS
                        "app_main.c"
                        "audio.c"
                        "${esp_idf_common}/shell.c"
                        "${esp_idf_common}/wifi.c"
                        "${esp_idf_common}/nvs.c"
                        "${esp_idf_common}/sample_credentials.c"
                        "${bsp_srcs}"

                    INCLUDE_DIRS
                        "${esp_idf_common}"
                        "${bsp_includes}"
                        "."

                    PRIV_REQUIRES
                        "golioth_sdk"
                        "console"
                        "fatfs"
                        "spi_flash"
                        "nvs_flash"
                        "json"
                        "driver"
                        "esp_hw_support"
                        "esp_wifi"
                        )
