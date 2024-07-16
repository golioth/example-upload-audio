#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SD_MOUNT_POINT      "/sdcard"

struct audio_ctx {
    char filename[32];
    uint32_t rec_time;
};

struct audio_ctx audio_ctx_default(void);
void record_wav(struct audio_ctx *a_ctx);
void init_microphone(void);
