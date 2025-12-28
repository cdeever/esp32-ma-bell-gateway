#include "audio_bridge.h"
#include "audio_output.h"
#include "config/audio_config.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "esp_hf_client_api.h"
#include <string.h>

static const char *TAG = "audio_bridge";

// FreeRTOS task handles
static TaskHandle_t audio_rx_task_handle = NULL;
static TaskHandle_t audio_tx_task_handle = NULL;

// Ring buffers for audio bridging
// RX buffer: Bluetooth → ESP32 → Phone (from BT incoming callback to I2S TX)
// TX buffer: Phone → ESP32 → Bluetooth (from I2S RX to BT outgoing callback)
static RingbufHandle_t bt_rx_ringbuf = NULL;  // Audio from Bluetooth
static RingbufHandle_t bt_tx_ringbuf = NULL;  // Audio to Bluetooth

// Audio frame size for 8kHz, 16-bit, 20ms = 160 samples * 2 bytes = 320 bytes
#define AUDIO_FRAME_SIZE 320
#define AUDIO_FRAME_DURATION_MS 20

/**
 * @brief Audio RX task - Reads audio from PCM1808 ADC and sends to Bluetooth
 *
 * This task continuously reads audio samples from the I2S RX channel (ADC)
 * and writes them to the Bluetooth TX ring buffer. The Bluetooth outgoing
 * callback will read from this buffer to send audio to the connected phone.
 */
static void audio_rx_task(void *arg)
{
    ESP_LOGI(TAG, "Audio RX task started (Phone → Bluetooth)");

    // Get RX handle from audio_output module
    i2s_chan_handle_t rx_handle = audio_output_get_rx_handle();
    if (rx_handle == NULL) {
        ESP_LOGE(TAG, "RX handle not available - audio_output not initialized?");
        vTaskDelete(NULL);
        return;
    }

    uint8_t i2s_rx_buffer[AUDIO_FRAME_SIZE];
    size_t bytes_read;

    while (1) {
        // Read audio from PCM1808 ADC via I2S RX
        esp_err_t ret = i2s_channel_read(rx_handle, i2s_rx_buffer,
                                          AUDIO_FRAME_SIZE, &bytes_read,
                                          portMAX_DELAY);

        if (ret == ESP_OK && bytes_read > 0) {
            // Write audio to Bluetooth TX ring buffer
            BaseType_t done = xRingbufferSend(bt_tx_ringbuf, i2s_rx_buffer,
                                               bytes_read, pdMS_TO_TICKS(10));
            if (!done) {
                ESP_LOGW(TAG, "BT TX ring buffer full, dropping audio frame");
            } else {
                // Notify Bluetooth stack that data is ready
                esp_hf_client_outgoing_data_ready();
            }
        } else {
            ESP_LOGW(TAG, "I2S RX read failed: %s", esp_err_to_name(ret));
        }

        // Small delay to match audio frame timing (20ms frames)
        vTaskDelay(pdMS_TO_TICKS(AUDIO_FRAME_DURATION_MS));
    }
}

/**
 * @brief Audio TX task - Reads audio from Bluetooth and writes to PCM5100 DAC
 *
 * This task continuously reads audio samples from the Bluetooth RX ring buffer
 * (filled by the Bluetooth incoming callback) and writes them to the I2S TX
 * channel (DAC) for playback on the phone handset speaker.
 */
static void audio_tx_task(void *arg)
{
    ESP_LOGI(TAG, "Audio TX task started (Bluetooth → Phone)");

    uint8_t bt_audio_buffer[AUDIO_FRAME_SIZE];
    size_t item_size = 0;

    while (1) {
        // Read audio from Bluetooth RX ring buffer
        uint8_t *data = xRingbufferReceiveUpTo(bt_rx_ringbuf, &item_size,
                                                 pdMS_TO_TICKS(100),
                                                 AUDIO_FRAME_SIZE);

        if (data != NULL && item_size > 0) {
            // Copy data to local buffer
            memcpy(bt_audio_buffer, data, item_size);
            vRingbufferReturnItem(bt_rx_ringbuf, data);

            // Write audio via audio_output module
            // If a tone is playing, this will be silently dropped (tone has priority)
            size_t bytes_written;
            esp_err_t ret = audio_output_write(bt_audio_buffer, item_size,
                                                &bytes_written, 100);

            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "Audio output write failed: %s", esp_err_to_name(ret));
            }
        }

        // Small delay to prevent task starvation
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

esp_err_t audio_bridge_init(void)
{
    ESP_LOGI(TAG, "Initializing audio bridge");

    // Verify I2S RX channel is available from audio_output module
    if (audio_output_get_rx_handle() == NULL) {
        ESP_LOGE(TAG, "I2S RX channel not available - call audio_output_init() first");
        return ESP_ERR_INVALID_STATE;
    }

    // Create ring buffers for audio bridging
    bt_rx_ringbuf = xRingbufferCreate(AUDIO_HFP_RINGBUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (bt_rx_ringbuf == NULL) {
        ESP_LOGE(TAG, "Failed to create BT RX ring buffer");
        return ESP_FAIL;
    }

    bt_tx_ringbuf = xRingbufferCreate(AUDIO_HFP_RINGBUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (bt_tx_ringbuf == NULL) {
        ESP_LOGE(TAG, "Failed to create BT TX ring buffer");
        vRingbufferDelete(bt_rx_ringbuf);
        bt_rx_ringbuf = NULL;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Audio bridge initialized (ring buffers ready)");
    return ESP_OK;
}

void audio_bridge_start(void)
{
    ESP_LOGI(TAG, "Starting audio bridge");

    if (audio_rx_task_handle == NULL) {
        BaseType_t ret = xTaskCreate(audio_rx_task, "audio_rx",
                                      4096, NULL, 10, &audio_rx_task_handle);
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create audio RX task");
            return;
        }
    }

    if (audio_tx_task_handle == NULL) {
        BaseType_t ret = xTaskCreate(audio_tx_task, "audio_tx",
                                      4096, NULL, 10, &audio_tx_task_handle);
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create audio TX task");
            return;
        }
    }

    ESP_LOGI(TAG, "Audio bridge started - bidirectional audio active");
}

void audio_bridge_stop(void)
{
    ESP_LOGI(TAG, "Stopping audio bridge");

    if (audio_rx_task_handle != NULL) {
        vTaskDelete(audio_rx_task_handle);
        audio_rx_task_handle = NULL;
    }

    if (audio_tx_task_handle != NULL) {
        vTaskDelete(audio_tx_task_handle);
        audio_tx_task_handle = NULL;
    }

    // Clear ring buffers
    if (bt_rx_ringbuf != NULL) {
        // Drain the buffer
        size_t item_size;
        uint8_t *data;
        while ((data = xRingbufferReceive(bt_rx_ringbuf, &item_size, 0)) != NULL) {
            vRingbufferReturnItem(bt_rx_ringbuf, data);
        }
    }

    if (bt_tx_ringbuf != NULL) {
        // Drain the buffer
        size_t item_size;
        uint8_t *data;
        while ((data = xRingbufferReceive(bt_tx_ringbuf, &item_size, 0)) != NULL) {
            vRingbufferReturnItem(bt_tx_ringbuf, data);
        }
    }

    ESP_LOGI(TAG, "Audio bridge stopped");
}

/**
 * @brief Get the Bluetooth RX ring buffer handle
 *
 * Used by bt_app_hf.c incoming callback to write received audio
 */
RingbufHandle_t audio_bridge_get_bt_rx_ringbuf(void)
{
    return bt_rx_ringbuf;
}

/**
 * @brief Get the Bluetooth TX ring buffer handle
 *
 * Used by bt_app_hf.c outgoing callback to read audio to send
 */
RingbufHandle_t audio_bridge_get_bt_tx_ringbuf(void)
{
    return bt_tx_ringbuf;
}
