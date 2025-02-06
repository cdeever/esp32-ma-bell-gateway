#include <stdio.h>
#include <math.h>
#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tones.h"

#define PLAY_DURATION 5 // Play each tone for 5 seconds


// Define the tones
tone_t tones[NUM_TONES] = {
    {350, 440, -1, -1},      // Dial Tone (continuous)
    {440, 480, 2, 4},        // Ringback Tone
    {480, 620, 0.5, 0.5},    // Busy Signal
    {480, 620, 0.25, 0.25},  // Reorder Tone (Fast Busy)
    {1400, 2060, 0.1, 0.1},  // Off-Hook Warning Tone
    {480, 620, 0.2, 0.3},    // Congestion Tone
    {350, 440, 0.1, 0.1},    // Confirmation Tone
    {440, 0, 0.3, -1},       // Call Waiting Tone
    {950, 1400, 0.274, 0},   // Special Information Tone (SIT)
    {350, 440, 0.1, 0.1}     // Stutter Dial Tone
};


void i2s_init() {
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 22,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_PORT, &pin_config);
}

void generate_tone_task(void *param) {
    tone_type_t tone_type = (tone_type_t) param;
    tone_t tone = tones[tone_type];
    int16_t buffer[BUFFER_SIZE];
    int samples_per_cycle1 = SAMPLE_RATE / tone.freq1;
    int samples_per_cycle2 = tone.freq2 ? SAMPLE_RATE / tone.freq2 : 0;
    int sample_index = 0;
    int elapsed_samples = 0;
    float volume_factor = 0.2;

    while (elapsed_samples < PLAY_DURATION * SAMPLE_RATE) {
        if (tone.duration_on == -1) { // Continuous tone
            for (int i = 0; i < BUFFER_SIZE; i++) {
                float sample1 = sinf(2.0f * M_PI * (i % samples_per_cycle1) / samples_per_cycle1);
                float sample2 = tone.freq2 ? sinf(2.0f * M_PI * (i % samples_per_cycle2) / samples_per_cycle2) : 0;
                buffer[sample_index++] = (int16_t)((32767 * volume_factor) * (sample1 + sample2) / 2);
                if (sample_index >= BUFFER_SIZE) {
                    size_t bytes_written;
                    i2s_write(I2S_PORT, buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY);
                    sample_index = 0;
                }
                elapsed_samples++;
            }
        } else { // Pulsed tone
            int total_samples = (int)(tone.duration_on * SAMPLE_RATE);
            for (int i = 0; i < total_samples; i++) {
                float sample1 = sinf(2.0f * M_PI * (i % samples_per_cycle1) / samples_per_cycle1);
                float sample2 = tone.freq2 ? sinf(2.0f * M_PI * (i % samples_per_cycle2) / samples_per_cycle2) : 0;
                buffer[sample_index++] = (int16_t)((32767 * volume_factor) * (sample1 + sample2) / 2);
                if (sample_index >= BUFFER_SIZE) {
                    size_t bytes_written;
                    i2s_write(I2S_PORT, buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY);
                    sample_index = 0;
                }
                elapsed_samples++;
            }
            elapsed_samples += tone.duration_off * SAMPLE_RATE;
            if (tone.duration_off > 0) {
                vTaskDelay(pdMS_TO_TICKS(tone.duration_off * 1000));
            }
        }
    }
    vTaskDelete(NULL);
}

