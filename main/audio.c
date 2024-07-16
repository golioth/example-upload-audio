/* Audio */
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_vfs.h"
#include "driver/i2s_pdm.h"
#include "format_wav.h"

#include "audio.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#include "m5stack_core2.h"
i2s_chan_handle_t rx_handle = NULL;
#endif /* CONFIG_IDF_TARGET_ESP32 */

#ifdef CONFIG_IDF_TARGET_ESP32S3
#include "esp_codec_dev.h"
#include "bsp/m5stack_core_s3.h"
static esp_codec_dev_handle_t mic_codec_dev = NULL;
#endif /* CONFIG_IDF_TARGET_ESP32S3 */

/* Include the Golioth Client to access backend logging */
#include <golioth/client.h>
static const char *TAG = "audio_and_sd";

#define SPI_DMA_CHAN        SPI_DMA_CH_AUTO
#define NUM_CHANNELS        (1) // For mono recording only!
#define FILENAME_DEFAULT    "record.wav"
#define SAMPLE_SIZE         (CONFIG_EXAMPLE_BIT_SAMPLE * 1024)
#define BYTE_RATE           (CONFIG_EXAMPLE_SAMPLE_RATE * (CONFIG_EXAMPLE_BIT_SAMPLE / 8)) * NUM_CHANNELS

static int16_t i2s_readraw_buff[SAMPLE_SIZE];
size_t bytes_read;
const int WAVE_HEADER_SIZE = 44;


struct audio_ctx audio_ctx_default(void)
{
    struct audio_ctx a_ctx;
    snprintf(a_ctx.filename, sizeof(a_ctx.filename), "%s", FILENAME_DEFAULT);
    a_ctx.rec_time = CONFIG_EXAMPLE_REC_TIME;

    return a_ctx;
}

static FILE *prep_recording(struct audio_ctx *a_ctx)
{
    char path[sizeof(SD_MOUNT_POINT) + sizeof(a_ctx->filename)];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, a_ctx->filename);

    // Use POSIX and C standard library functions to work with files.
    GLTH_LOGI(TAG, "Opening file: %s", path);

    uint32_t flash_rec_time = BYTE_RATE * a_ctx->rec_time;
    const wav_header_t wav_header =
        WAV_HEADER_PCM_DEFAULT(flash_rec_time, 16, CONFIG_EXAMPLE_SAMPLE_RATE, 1);

    // First check if file exists before creating a new file.
    struct stat st;
    if (stat(path, &st) == 0) {
        // Delete it if it exists
        unlink(path);
    }

    // Create new WAV file
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        GLTH_LOGE(TAG, "Failed to open file for writing");
        return NULL;
    }

    // Write the header to the WAV file
    fwrite(&wav_header, sizeof(wav_header), 1, f);
    return f;
}


#ifdef CONFIG_IDF_TARGET_ESP32

void init_microphone(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));

    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(CONFIG_EXAMPLE_SAMPLE_RATE),
        /* The default mono slot is the left slot (whose 'select pin' of the PDM microphone is pulled down) */
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = CONFIG_EXAMPLE_I2S_CLK_GPIO,
            .din = CONFIG_EXAMPLE_I2S_DATA_GPIO,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_handle, &pdm_rx_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
}

void record_wav(struct audio_ctx *a_ctx)
{
    int flash_wr_size = 0;
    uint32_t flash_rec_time = BYTE_RATE * a_ctx->rec_time;

    FILE *f = prep_recording(a_ctx);
    if (!f)
    {
        GLTH_LOGE(TAG, "Recording unsuccessful");
        return;
    }

    // Start recording
    while (flash_wr_size < flash_rec_time) {
        // Read the RAW samples from the microphone
        if (i2s_channel_read(rx_handle, (char *)i2s_readraw_buff, SAMPLE_SIZE, &bytes_read, 1000) == ESP_OK) {
            printf("[0] %d [1] %d [2] %d [3]%d ...\n", i2s_readraw_buff[0], i2s_readraw_buff[1], i2s_readraw_buff[2], i2s_readraw_buff[3]);
            // Write the samples to the WAV file
            fwrite(i2s_readraw_buff, bytes_read, 1, f);
            flash_wr_size += bytes_read;
        } else {
            printf("Read Failed!\n");
        }
    }

    GLTH_LOGI(TAG, "Recording done!");
    fclose(f);
    GLTH_LOGI(TAG, "File written on SDCard");
}
#endif /* CONFIG_IDF_TARGET_ESP32 */


#ifdef CONFIG_IDF_TARGET_ESP32S3

void init_microphone(void)
{
    mic_codec_dev = bsp_audio_codec_microphone_init();
    esp_codec_dev_set_in_gain(mic_codec_dev, 42.0);
}

void record_wav(struct audio_ctx *a_ctx)
{
    int flash_wr_size = 0;
    uint32_t flash_rec_time = BYTE_RATE * a_ctx->rec_time;

    FILE *f = prep_recording(a_ctx);
    if (!f)
    {
        GLTH_LOGE(TAG, "Recording unsuccessful");
        return;
    }

    // Open codec
    esp_codec_dev_sample_info_t codec_record_cfg = {
        .bits_per_sample = CONFIG_EXAMPLE_BIT_SAMPLE,
        .channel = 1,
        .sample_rate = CONFIG_EXAMPLE_SAMPLE_RATE,
    };

    int err = esp_codec_dev_open(mic_codec_dev, &codec_record_cfg);
    if (err != ESP_CODEC_DEV_OK)
    {
        GLTH_LOGE(TAG, "Unable to open mic codec %d", err);
        return;
    }

    // Start recording
    while (flash_wr_size < flash_rec_time) {
        if (esp_codec_dev_read(mic_codec_dev, (char *)i2s_readraw_buff, SAMPLE_SIZE) == ESP_CODEC_DEV_OK) {
            printf("[0] %d [1] %d [2] %d [3]%d ...\n", i2s_readraw_buff[0], i2s_readraw_buff[1], i2s_readraw_buff[2], i2s_readraw_buff[3]);
            // Write the samples to the WAV file
            fwrite(i2s_readraw_buff, SAMPLE_SIZE, 1, f);
            flash_wr_size += SAMPLE_SIZE;
        } else {
            printf("Read Failed!\n");
        }
    }

    GLTH_LOGI(TAG, "Recording done!");
    fclose(f);

    err = esp_codec_dev_close(mic_codec_dev);
    if (err == ESP_CODEC_DEV_INVALID_ARG)
    {
        GLTH_LOGE(TAG, "Invalid arg when closing mic codec %d", err);
        return;
    }
}
#endif /* CONFIG_IDF_TARGET_ESP32S3 */
