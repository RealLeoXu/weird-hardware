#include "I2SOutput.h"

#include <freertos/FreeRTOS.h>

// the S3 has no APLL, the old driver with use_apll gave a noisy clock
#if __has_include(<driver/i2s_std.h>)
#define AMBIENCE_USE_I2S_STD 1
#include <driver/i2s_std.h>
#else
#define AMBIENCE_USE_I2S_STD 0
#include <driver/i2s.h>
#endif

#if AMBIENCE_USE_I2S_STD

namespace {
i2s_chan_handle_t g_txHandle = nullptr;
}

bool I2SOutput::begin(int sampleRate) {
    i2s_chan_config_t chanConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    if (i2s_new_channel(&chanConfig, &g_txHandle, nullptr) != ESP_OK) {
        return false;
    }

    i2s_std_config_t stdConfig = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(static_cast<uint32_t>(sampleRate)),

        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = static_cast<gpio_num_t>(I2S_BCLK_PIN),
            .ws = static_cast<gpio_num_t>(I2S_LRCLK_PIN),
            .dout = static_cast<gpio_num_t>(I2S_DOUT_PIN),
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    if (i2s_channel_init_std_mode(g_txHandle, &stdConfig) != ESP_OK) {
        return false;
    }

    return i2s_channel_enable(g_txHandle) == ESP_OK;
}

void I2SOutput::writeInterleaved(const int16_t* interleaved, size_t frameCount) {
    size_t bytesWritten = 0;
    const size_t bytesToWrite = frameCount * 2  * sizeof(int16_t);
    i2s_channel_write(g_txHandle, interleaved, bytesToWrite, &bytesWritten, portMAX_DELAY);
}

#else

namespace {
constexpr i2s_port_t kI2SPort = I2S_NUM_0;
}

bool I2SOutput::begin(int sampleRate) {
    i2s_config_t config = {};
    config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
    config.sample_rate = static_cast<uint32_t>(sampleRate);
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S);
    config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    config.dma_buf_count = 8;
    config.dma_buf_len = 256;
#if CONFIG_IDF_TARGET_ESP32
    config.use_apll = true;
#else
    config.use_apll = false;
#endif

    esp_err_t err = i2s_driver_install(kI2SPort, &config, 0, nullptr);
    if (err != ESP_OK) {
        return false;
    }

    i2s_pin_config_t pins = {};
    pins.bck_io_num = I2S_BCLK_PIN;
    pins.ws_io_num = I2S_LRCLK_PIN;
    pins.data_out_num = I2S_DOUT_PIN;
    pins.data_in_num = I2S_PIN_NO_CHANGE;

    err = i2s_set_pin(kI2SPort, &pins);
    return err == ESP_OK;
}

void I2SOutput::writeInterleaved(const int16_t* interleaved, size_t frameCount) {
    size_t bytesWritten = 0;
    const size_t bytesToWrite = frameCount * 2  * sizeof(int16_t);
    i2s_write(kI2SPort, interleaved, bytesToWrite, &bytesWritten, portMAX_DELAY);
}

#endif
