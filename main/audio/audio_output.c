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

// Largest write audio_output_write() takes at once (the bridge sends 160-sample frames)
#define AUDIO_WRITE_MAX_SAMPLES 256

// Tone level as a fraction of full scale (0.0 to 1.0); see audio_output_tone_level_step()
#define TONE_VOLUME_DEFAULT 0.141f  // -17 dBFS: clean on the MAX98357A bench speaker at 9 dB gain
#define TONE_VOLUME_MAX     1.0f
#define TONE_VOLUME_MIN     0.01f
static volatile float tone_volume = TONE_VOLUME_DEFAULT;

// Pre-computed values for efficiency
#define TWO_PI (2.0f * M_PI)

/**
 * @brief Write 16-bit mono samples to I2S in the order the ESP32 needs
 *
 * In 16-bit mono mode the ESP32's I2S sends each pair of samples swapped
 * (written 1,2,3,4 goes out as 2,1,4,3; see "16 bit mono" in the ESP-IDF I2S
 * docs). Swapping each pair here puts them back in order on the line.
 * Uncorrected, every tone gets an image near 4 kHz and sounds rough.
 * Swaps the caller's buffer in place; a trailing odd sample is dropped.
 */
static esp_err_t i2s_write_mono16(int16_t *samples, size_t count, size_t *bytes_written,
                                  TickType_t timeout)
{
    count &= ~(size_t)1;
    for (size_t i = 0; i < count; i += 2) {
        int16_t t = samples[i];
        samples[i] = samples[i + 1];
        samples[i + 1] = t;
    }
    return i2s_channel_write(tx_handle, samples, count * sizeof(int16_t), bytes_written, timeout);
}

// Oscillator state carried from one buffer to the next
typedef struct {
    float phase1;
    float phase2;
    int warble_count;  // Samples into the current warble half-cycle
    bool warble_high;  // Warble: true while freq2 is sounding
} tone_osc_t;

static bool tone_changed(tone_type_t playing)
{
    xSemaphoreTake(tone_mutex, portMAX_DELAY);
    bool changed = (current_tone != playing);
    xSemaphoreGive(tone_mutex);
    return changed;
}

/**
 * @brief Fill a buffer with a tone's samples
 *
 * A two-frequency tone sounds both frequencies at once. A warble tone
 * (warble_hz set) sounds one at a time, switching between them on a single
 * phase-continuous oscillator so the switch doesn't click.
 */
static void fill_tone(int16_t *buffer, int count, const tone_t *tone, tone_osc_t *osc)
{
    const float volume = tone_volume;
    const float inc1 = TWO_PI * tone->freq1 / AUDIO_SAMPLE_RATE;
    const float inc2 = TWO_PI * tone->freq2 / AUDIO_SAMPLE_RATE;

    if (tone->warble_hz > 0) {
        // Each frequency sounds for half a warble cycle
        const int half_cycle = AUDIO_SAMPLE_RATE / (2 * tone->warble_hz);
        for (int i = 0; i < count; i++) {
            buffer[i] = (int16_t)(32767.0f * volume * sinf(osc->phase1));
            osc->phase1 += osc->warble_high ? inc2 : inc1;
            if (osc->phase1 >= TWO_PI) osc->phase1 -= TWO_PI;
            if (++osc->warble_count >= half_cycle) {
                osc->warble_count = 0;
                osc->warble_high = !osc->warble_high;
            }
        }
        return;
    }

    for (int i = 0; i < count; i++) {
        float sample1 = sinf(osc->phase1);
        float sample2 = tone->freq2 ? sinf(osc->phase2) : 0.0f;
        float mixed = (sample1 + sample2) / 2.0f;
        buffer[i] = (int16_t)(32767.0f * volume * mixed);

        osc->phase1 += inc1;
        if (osc->phase1 >= TWO_PI) osc->phase1 -= TWO_PI;
        osc->phase2 += inc2;
        if (osc->phase2 >= TWO_PI) osc->phase2 -= TWO_PI;
    }
}

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
    tone_osc_t osc = { 0 };

    while (1) {
        // Check if we have a tone to play
        xSemaphoreTake(tone_mutex, portMAX_DELAY);
        tone_type_t tone_to_play = current_tone;
        xSemaphoreGive(tone_mutex);

        if (tone_to_play == TONE_NONE) {
            // No tone active, sleep briefly
            vTaskDelay(pdMS_TO_TICKS(50));
            osc = (tone_osc_t){ 0 };
            continue;
        }

        // Get tone parameters
        const tone_t *tone = tone_get_definition(tone_to_play);
        if (tone == NULL) {
            ESP_LOGW(TAG, "Invalid tone type: %d", tone_to_play);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (tone->duration_on < 0) {
            // Continuous tone - generate until stopped or changed
            while (!tone_changed(tone_to_play)) {
                fill_tone(buffer, AUDIO_BUFFER_SIZE, tone, &osc);

                // Write to I2S
                size_t bytes_written;
                esp_err_t ret = i2s_write_mono16(buffer, AUDIO_BUFFER_SIZE,
                                                 &bytes_written, portMAX_DELAY);
                if (ret != ESP_OK) {
                    ESP_LOGW(TAG, "I2S write failed: %s", esp_err_to_name(ret));
                }
            }
        } else {
            // Pulsed tone - on/off cadence
            int samples_on = (int)(tone->duration_on * AUDIO_SAMPLE_RATE);
            int samples_off = (int)(tone->duration_off * AUDIO_SAMPLE_RATE);

            while (!tone_changed(tone_to_play)) {
                // Generate "on" phase. Checked each buffer, so a long burst
                // (ringing is 2 s) stops promptly when the call is answered.
                int samples_generated = 0;
                while (samples_generated < samples_on) {
                    int samples_this_buffer = AUDIO_BUFFER_SIZE;
                    if (samples_generated + samples_this_buffer > samples_on) {
                        samples_this_buffer = samples_on - samples_generated;
                    }

                    fill_tone(buffer, samples_this_buffer, tone, &osc);

                    size_t bytes_written;
                    i2s_write_mono16(buffer, samples_this_buffer, &bytes_written, portMAX_DELAY);
                    samples_generated += samples_this_buffer;

                    if (tone_changed(tone_to_play)) {
                        goto next_tone;
                    }
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

        // Reset the oscillator for the next tone
        osc = (tone_osc_t){ 0 };
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

    // Copy so the pair swap doesn't change the caller's buffer
    int16_t samples[AUDIO_WRITE_MAX_SAMPLES];
    size_t count = len / sizeof(int16_t);
    if (count > AUDIO_WRITE_MAX_SAMPLES) {
        count = AUDIO_WRITE_MAX_SAMPLES;
    }
    memcpy(samples, data, count * sizeof(int16_t));
    return i2s_write_mono16(samples, count, bytes_written, pdMS_TO_TICKS(timeout_ms));
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

float audio_output_tone_level_step(int steps)
{
    // Each step is 3 dB (a factor of sqrt(2) in amplitude)
    float v = tone_volume;
    for (; steps > 0; steps--) v *= 1.41421356f;
    for (; steps < 0; steps++) v /= 1.41421356f;
    if (v > TONE_VOLUME_MAX) v = TONE_VOLUME_MAX;
    if (v < TONE_VOLUME_MIN) v = TONE_VOLUME_MIN;
    tone_volume = v;
    float db = 20.0f * log10f(v);
    ESP_LOGI(TAG, "Tone level %.3f of full scale (%.1f dBFS)", v, db);
    return db;
}
