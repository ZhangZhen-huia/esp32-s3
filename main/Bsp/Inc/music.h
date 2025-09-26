#pragma once
#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "Inc/i2c.h"
#include "Inc/dev_registry.h"

#define VOLUME_DEFAULT    60

#define CODEC_DEFAULT_SAMPLE_RATE          (48000)
#define CODEC_DEFAULT_BIT_WIDTH            (16)
#define CODEC_DEFAULT_ADC_VOLUME           (24.0)
#define CODEC_DEFAULT_CHANNEL              (2)

#define BSP_I2S_NUM                  I2S_NUM_1

#define GPIO_I2S_LRCK       (GPIO_NUM_13)
#define GPIO_I2S_MCLK       (GPIO_NUM_38)
#define GPIO_I2S_SCLK       (GPIO_NUM_14)
#define GPIO_I2S_SDIN       (GPIO_NUM_12)
#define GPIO_I2S_DOUT       (GPIO_NUM_45)
#define GPIO_PWR_CTRL       (GPIO_NUM_NC)
esp_err_t bsp_codec_init(void);
esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);
esp_err_t bsp_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);
esp_err_t bsp_codec_mute_set(bool enable);
esp_err_t bsp_codec_volume_set(int volume, int *volume_set);
void register_music_device(void);