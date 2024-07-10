set(esp_idf_common "../submodules/golioth-firmware-sdk/examples/esp_idf/common")

idf_component_register(SRCS
                        "app_main.c"
                        "${esp_idf_common}/shell.c"
                        "${esp_idf_common}/wifi.c"
                        "${esp_idf_common}/nvs.c"
                        "${esp_idf_common}/sample_credentials.c"

                    INCLUDE_DIRS
                        "${esp_idf_common}"

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
