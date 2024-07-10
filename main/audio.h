#pragma once

#include <stdint.h>

struct audio_ctx {
    char filename[32];
    uint32_t rec_time;
};

void mount_sdcard(void);
struct audio_ctx audio_ctx_default(void);
void record_wav(struct audio_ctx *a_ctx);
void unmount_and_free(void);
void init_microphone(void);
