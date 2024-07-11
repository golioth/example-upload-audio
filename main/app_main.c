/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

static const char *TAG = "golioth_audio_upload";

/* Devic PMU Control */
#include "m5stack_core2.h"

/* Microphone and SD Card */
#include <stdio.h>
#include <sys/stat.h>
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

FILE *get_audio_filestream(struct audio_ctx *a_ctx)
{
    char path[sizeof(SD_MOUNT_POINT) + sizeof(a_ctx->filename)];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, a_ctx->filename);

    struct stat st;
    if (stat(path, &st) != 0) {
        GLTH_LOGE(TAG, "File not found");
        return NULL;
    }
    else {
        GLTH_LOGI(TAG, "File size: %li", st.st_size);
    }

    GLTH_LOGI(TAG, "Opening file: %s", path);

    FILE *f = fopen(SD_MOUNT_POINT"/record.wav", "r");
    if (!f) {
        GLTH_LOGE(TAG, "Failed to open file for reading");
        return NULL;
    }

    return f;
}

void release_audio_filestream(FILE *f)
{
    if (!f)
    {
        GLTH_LOGE(TAG, "Filestream is NULL");
        return;
    }

    fclose(f);
}

enum golioth_status block_upload_audio_filestream_cb(uint32_t block_idx,
                                                     uint8_t *block_buffer,
                                                     size_t *block_size,
                                                     bool *is_last,
                                                     void *arg)
{
    int err = 0;
    FILE *f = (FILE *)arg;

    if (!f)
    {
        GLTH_LOGE(TAG, "arg was NULL but should have been pointer to a filestream");
        return GOLIOTH_ERR_INVALID_STATE;
    }

    size_t bytes_read = fread(block_buffer, 1, *block_size, f);

    err = ferror(f);
    if (err)
    {
        GLTH_LOGE(TAG, "Error reading filestream: %d", err);
        return ESP_ERR_INVALID_STATE;
    }

    if (bytes_read < *block_size)
    {
        *block_size = bytes_read;
    }

    int eof = feof(f);
    if (eof)
    {
        *is_last = true;
    }

    if (*block_size == 0)
    {
        GLTH_LOGE(TAG, "Error, no bytes read from audio filestream");
        goto error_uploading_file;
    }

    GLTH_LOGI(TAG,
              "Uploading block_id: %u block_size: %zu is_last: %u",
              (unsigned int) block_idx,
              *block_size,
              *is_last);

    return GOLIOTH_OK;

error_uploading_file:
    *block_size = 0;
    *is_last = 1;
    return GOLIOTH_ERR_NO_MORE_DATA;
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

    /* Record Audio */
    struct audio_ctx a_ctx = audio_ctx_default();
    mount_sdcard();
    init_microphone();

    GLTH_LOGI(TAG, "Starting recording for %" PRIu32 " seconds!", a_ctx.rec_time);

    record_wav(&a_ctx);

    /* Stream to Golioth */
    FILE *f = get_audio_filestream(&a_ctx);

    int err = golioth_stream_set_blockwise_sync(client,
                                                "file_upload",
                                                GOLIOTH_CONTENT_TYPE_OCTET_STREAM,
                                                block_upload_audio_filestream_cb,
                                                (void *) f);
    if (err)
    {
        GLTH_LOGE(TAG, "Failed to upload file: %d", err);
    }
    else
    {
        GLTH_LOGI(TAG, "Upload successful!");
    }

    release_audio_filestream(f);

    /* Unmount and disable SD card */
    unmount_and_free();
}
