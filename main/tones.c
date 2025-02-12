#include <stdio.h>
#include <math.h>
#include <string.h>
#include "driver/i2s_std.h"
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

volatile tone_type_t current_tone = NUM_TONES;  // Default: no tone playing

i2s_chan_handle_t tx_handle;  // Handle for the I2S TX channel

void i2s_init(void) {
    // 1️⃣ Create the I2S TX channel
    i2s_chan_config_t i2s_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&i2s_chan_cfg, &tx_handle, NULL));

    // 2️⃣ Configure standard I2S settings (GPIO, clock, slot format)
    i2s_std_config_t i2s_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),  // 16kHz sample rate
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_0,   // Set MCLK if needed
            .bclk = GPIO_NUM_26,  // Replace with actual BCLK pin
            .ws   = GPIO_NUM_25,  // Replace with actual WS (LRCLK) pin
            .dout = GPIO_NUM_22,  // Replace with actual Data Out pin
            .din  = I2S_GPIO_UNUSED,  // Not needed for TX-only mode
        }
    };

    // 3️⃣ Apply the standard configuration to the I2S channel
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &i2s_std_cfg));

    // 4️⃣ Enable the I2S channel
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
}


void generate_tone_task(void *param) {

    while (1) {

        if (current_tone == NUM_TONES) {
            vTaskDelay(pdMS_TO_TICKS(100));  // Sleep if no tone is set
            continue;
        }

        tone_t tone = tones[current_tone];  // Get the active tone
    
        //PLAY TONE!
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
                        ESP_ERROR_CHECK(i2s_channel_write(tx_handle, buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY));
                        sample_index = 0;
                    }
                    elapsed_samples++;
                }
            } else { // Pulsed tone
               // 🔊 Generate "On" Phase
                int total_samples_on = (int)(tone.duration_on * SAMPLE_RATE);
                for (int i = 0; i < total_samples_on; i++) {
                    float sample1 = sinf(2.0f * M_PI * (i % samples_per_cycle1) / samples_per_cycle1);
                    float sample2 = tone.freq2 ? sinf(2.0f * M_PI * (i % samples_per_cycle2) / samples_per_cycle2) : 0;
                    buffer[sample_index++] = (int16_t)((32767 * volume_factor) * (sample1 + sample2) / 2);

                    if (sample_index >= BUFFER_SIZE) {
                        size_t bytes_written;
                        ESP_ERROR_CHECK(i2s_channel_write(tx_handle, buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY));
                        sample_index = 0;
                    }
                }

                // 🔇 Generate "Off" Phase (Silence)
                int total_samples_off = (int)(tone.duration_off * SAMPLE_RATE);
                memset(buffer, 0, BUFFER_SIZE * sizeof(int16_t));  // Fill buffer with silence

                for (int i = 0; i < total_samples_off; i += BUFFER_SIZE) {
                    size_t bytes_written;
                    ESP_ERROR_CHECK(i2s_channel_write(tx_handle, buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY));
                }

                // 💤 Delay for off time (ensuring sync)
                vTaskDelay(pdMS_TO_TICKS(tone.duration_off * 1000));
            }
        }
    }
    vTaskDelete(NULL);
}

