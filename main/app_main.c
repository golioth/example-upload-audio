/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

static const char *TAG = "golioth_audio_upload";

/* Devic PMU Control */
#include "m5stack_core2.h"

/* Microphone and SD Card */
#include "audio.h"

/* Golioth */
#include "nvs.h"
#include "shell.h"
#include "wifi.h"
#include "sample_credentials.h"
#include <golioth/client.h>
#include <golioth/stream.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static SemaphoreHandle_t _connected_sem = NULL;

static void on_client_event(struct golioth_client *client,
                            enum golioth_client_event event,
                            void *arg)
{
    bool is_connected = (event == GOLIOTH_CLIENT_EVENT_CONNECTED);
    if (is_connected)
    {
        xSemaphoreGive(_connected_sem);
    }
    GLTH_LOGI(TAG, "Golioth client %s", is_connected ? "connected" : "disconnected");
}

void app_main(void)
{
    GLTH_LOGI(TAG, "Start Golioth upload audio example");

    /* Initialize PMU */
    m5stack_core2_init_pmu();

    /* Golioth connection */
    /* Get credentials from NVS and enable shell */
    nvs_init();
    shell_start();

    if (!nvs_credentials_are_set())
    {
        GLTH_LOGW(TAG,
                 "WiFi and Golioth credentials are not set. "
                 "Use the shell settings commands to set them.");

        while (!nvs_credentials_are_set())
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    /* Initialize WiFi and wait for it to connect */
    wifi_init(nvs_read_wifi_ssid(), nvs_read_wifi_password());
    wifi_wait_for_connected();

    /* Connect to Golioth */
    const struct golioth_client_config *config = golioth_sample_credentials_get();
    struct golioth_client *client = golioth_client_create(config);
    _connected_sem = xSemaphoreCreateBinary();
    golioth_client_register_event_callback(client, on_client_event, NULL);

    GLTH_LOGW(TAG, "Waiting for connection to Golioth...");
    xSemaphoreTake(_connected_sem, portMAX_DELAY);

    int counter = 0;

    /* Record Audio */
    struct audio_ctx a_ctx = audio_ctx_default();
    mount_sdcard();
    init_microphone();

    GLTH_LOGI(TAG, "Starting recording for %"PRIu32" seconds!", a_ctx.rec_time);

    record_wav(&a_ctx);

    /* TODO: Stream to Golioth */

    /* Unmount and disable SD card */
    unmount_and_free();

    while (true)
    {
        GLTH_LOGI(TAG, "Sending hello! %d", counter);
        ++counter;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
