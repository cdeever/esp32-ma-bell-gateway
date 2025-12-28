#include "audio_output.h"
#include "config/audio_config.h"
#include "config/pin_assignments.h"
#include "tones.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <math.h>
#include <string.h>

static const char *TAG = "audio_output";

// I2S channel handles
// TX: Used for audio output (DAC - tones and BT audio to phone speaker)
// RX: Used for audio input (ADC - phone microphone to Bluetooth)
static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

// Tone state - protected by mutex
static SemaphoreHandle_t tone_mutex = NULL;
static volatile tone_type_t current_tone = TONE_NONE;
static TaskHandle_t tone_task_handle = NULL;

// Volume factor for tone generation (0.0 to 1.0)
#define TONE_VOLUME 0.2f

// Pre-computed values for efficiency
#define TWO_PI (2.0f * M_PI)

/**
 * @brief Tone generation task
 *
 * Runs continuously, generating audio samples when a tone is active.
 * Uses sine wave synthesis for dual-frequency tones.
 */
static void tone_generation_task(void *arg)
{
    ESP_LOGI(TAG, "Tone generation task started");

    int16_t buffer[AUDIO_BUFFER_SIZE];
    float phase1 = 0.0f;
    float phase2 = 0.0f;

    while (1) {
        // Check if we have a tone to play
        xSemaphoreTake(tone_mutex, portMAX_DELAY);
        tone_type_t tone_to_play = current_tone;
        xSemaphoreGive(tone_mutex);

        if (tone_to_play == TONE_NONE) {
            // No tone active, sleep briefly
            vTaskDelay(pdMS_TO_TICKS(50));
            phase1 = 0.0f;
            phase2 = 0.0f;
            continue;
        }

        // Get tone parameters
        const tone_t *tone = tone_get_definition(tone_to_play);
        if (tone == NULL) {
            ESP_LOGW(TAG, "Invalid tone type: %d", tone_to_play);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Calculate phase increments per sample
        float phase_inc1 = TWO_PI * tone->freq1 / AUDIO_SAMPLE_RATE;
        float phase_inc2 = tone->freq2 ? (TWO_PI * tone->freq2 / AUDIO_SAMPLE_RATE) : 0.0f;

        if (tone->duration_on < 0) {
            // Continuous tone - generate until stopped or changed
            while (1) {
                // Check if tone changed
                xSemaphoreTake(tone_mutex, portMAX_DELAY);
                if (current_tone != tone_to_play) {
                    xSemaphoreGive(tone_mutex);
                    break;
                }
                xSemaphoreGive(tone_mutex);

                // Generate buffer of samples
                for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
                    float sample1 = sinf(phase1);
                    float sample2 = tone->freq2 ? sinf(phase2) : 0.0f;
                    float mixed = (sample1 + sample2) / 2.0f;
                    buffer[i] = (int16_t)(32767.0f * TONE_VOLUME * mixed);

                    phase1 += phase_inc1;
                    if (phase1 >= TWO_PI) phase1 -= TWO_PI;
                    phase2 += phase_inc2;
                    if (phase2 >= TWO_PI) phase2 -= TWO_PI;
                }

                // Write to I2S
                size_t bytes_written;
                esp_err_t ret = i2s_channel_write(tx_handle, buffer,
                                                   AUDIO_BUFFER_SIZE * sizeof(int16_t),
                                                   &bytes_written, portMAX_DELAY);
                if (ret != ESP_OK) {
                    ESP_LOGW(TAG, "I2S write failed: %s", esp_err_to_name(ret));
                }
            }
        } else {
            // Pulsed tone - on/off cadence
            int samples_on = (int)(tone->duration_on * AUDIO_SAMPLE_RATE);
            int samples_off = (int)(tone->duration_off * AUDIO_SAMPLE_RATE);

            while (1) {
                // Check if tone changed
                xSemaphoreTake(tone_mutex, portMAX_DELAY);
                if (current_tone != tone_to_play) {
                    xSemaphoreGive(tone_mutex);
                    break;
                }
                xSemaphoreGive(tone_mutex);

                // Generate "on" phase
                int samples_generated = 0;
                while (samples_generated < samples_on) {
                    int samples_this_buffer = AUDIO_BUFFER_SIZE;
                    if (samples_generated + samples_this_buffer > samples_on) {
                        samples_this_buffer = samples_on - samples_generated;
                    }

                    for (int i = 0; i < samples_this_buffer; i++) {
                        float sample1 = sinf(phase1);
                        float sample2 = tone->freq2 ? sinf(phase2) : 0.0f;
                        float mixed = (sample1 + sample2) / 2.0f;
                        buffer[i] = (int16_t)(32767.0f * TONE_VOLUME * mixed);

                        phase1 += phase_inc1;
                        if (phase1 >= TWO_PI) phase1 -= TWO_PI;
                        phase2 += phase_inc2;
                        if (phase2 >= TWO_PI) phase2 -= TWO_PI;
                    }

                    size_t bytes_written;
                    i2s_channel_write(tx_handle, buffer,
                                      samples_this_buffer * sizeof(int16_t),
                                      &bytes_written, portMAX_DELAY);
                    samples_generated += samples_this_buffer;
                }

                // Generate "off" phase (silence)
                if (samples_off > 0) {
                    memset(buffer, 0, AUDIO_BUFFER_SIZE * sizeof(int16_t));
                    samples_generated = 0;
                    while (samples_generated < samples_off) {
                        int samples_this_buffer = AUDIO_BUFFER_SIZE;
                        if (samples_generated + samples_this_buffer > samples_off) {
                            samples_this_buffer = samples_off - samples_generated;
                        }

                        size_t bytes_written;
                        i2s_channel_write(tx_handle, buffer,
                                          samples_this_buffer * sizeof(int16_t),
                                          &bytes_written, portMAX_DELAY);
                        samples_generated += samples_this_buffer;

                        // Check if tone changed during silence
                        xSemaphoreTake(tone_mutex, portMAX_DELAY);
                        if (current_tone != tone_to_play) {
                            xSemaphoreGive(tone_mutex);
                            goto next_tone;
                        }
                        xSemaphoreGive(tone_mutex);
                    }
                }
            }
            next_tone:;
        }

        // Reset phases for next tone
        phase1 = 0.0f;
        phase2 = 0.0f;
    }
}

esp_err_t audio_output_init(void)
{
    ESP_LOGI(TAG, "Initializing audio output subsystem");

    // Create mutex for tone state
    tone_mutex = xSemaphoreCreateMutex();
    if (tone_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create tone mutex");
        return ESP_ERR_NO_MEM;
    }

    // I2S channel configuration - create both TX and RX channels together
    // This is required by ESP-IDF: both channels on same port must be created in single call
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(AUDIO_I2S_PORT, I2S_ROLE_MASTER);

    // Create both TX and RX channels in one call
    esp_err_t ret = i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channels: %s", esp_err_to_name(ret));
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ret;
    }

    // I2S standard (Philips) configuration for TX
    // MONO mode for telephony audio (8kHz, 16-bit)
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = PIN_PCM_CLK_OUT,
            .ws   = PIN_PCM_FSYNC,
            .dout = PIN_PCM_DOUT,
            .din  = GPIO_NUM_NC,  // TX doesn't use din
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // I2S standard configuration for RX (same settings as TX for consistency)
    i2s_std_config_t rx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = PIN_PCM_CLK_OUT,
            .ws   = PIN_PCM_FSYNC,
            .dout = GPIO_NUM_NC,  // RX doesn't use dout
            .din  = PIN_PCM_DIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // Initialize TX channel
    ret = i2s_channel_init_std_mode(tx_handle, &tx_std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S TX channel: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        i2s_del_channel(rx_handle);
        tx_handle = NULL;
        rx_handle = NULL;
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ret;
    }

    // Initialize RX channel
    ret = i2s_channel_init_std_mode(rx_handle, &rx_std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S RX channel: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        i2s_del_channel(rx_handle);
        tx_handle = NULL;
        rx_handle = NULL;
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ret;
    }

    // Enable TX channel
    ret = i2s_channel_enable(tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S TX channel: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        i2s_del_channel(rx_handle);
        tx_handle = NULL;
        rx_handle = NULL;
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ret;
    }

    // Enable RX channel
    ret = i2s_channel_enable(rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S RX channel: %s", esp_err_to_name(ret));
        i2s_channel_disable(tx_handle);
        i2s_del_channel(tx_handle);
        i2s_del_channel(rx_handle);
        tx_handle = NULL;
        rx_handle = NULL;
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ret;
    }

    // Create tone generation task
    BaseType_t xret = xTaskCreate(tone_generation_task, "tone_gen", 4096, NULL, 5, &tone_task_handle);
    if (xret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create tone generation task");
        i2s_channel_disable(tx_handle);
        i2s_channel_disable(rx_handle);
        i2s_del_channel(tx_handle);
        i2s_del_channel(rx_handle);
        tx_handle = NULL;
        rx_handle = NULL;
        vSemaphoreDelete(tone_mutex);
        tone_mutex = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Audio I/O initialized (TX: GPIO%d, RX: GPIO%d)", PIN_PCM_DOUT, PIN_PCM_DIN);
    return ESP_OK;
}

esp_err_t audio_output_write(const void *data, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    if (tx_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // If a tone is playing, block or return
    if (audio_output_tone_active()) {
        *bytes_written = 0;
        return ESP_OK;  // Silently drop - tone has priority
    }

    return i2s_channel_write(tx_handle, data, len, bytes_written,
                              pdMS_TO_TICKS(timeout_ms));
}

i2s_chan_handle_t audio_output_get_rx_handle(void)
{
    return rx_handle;
}

esp_err_t audio_output_play_tone(tone_type_t tone)
{
    if (tone_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (tone >= NUM_TONES && tone != TONE_NONE) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(tone_mutex, portMAX_DELAY);
    current_tone = tone;
    xSemaphoreGive(tone_mutex);

    ESP_LOGI(TAG, "Playing tone: %d", tone);
    return ESP_OK;
}

esp_err_t audio_output_stop_tone(void)
{
    return audio_output_play_tone(TONE_NONE);
}

bool audio_output_tone_active(void)
{
    if (tone_mutex == NULL) {
        return false;
    }

    xSemaphoreTake(tone_mutex, portMAX_DELAY);
    bool active = (current_tone != TONE_NONE);
    xSemaphoreGive(tone_mutex);

    return active;
}

tone_type_t audio_output_get_current_tone(void)
{
    if (tone_mutex == NULL) {
        return TONE_NONE;
    }

    xSemaphoreTake(tone_mutex, portMAX_DELAY);
    tone_type_t tone = current_tone;
    xSemaphoreGive(tone_mutex);

    return tone;
}
