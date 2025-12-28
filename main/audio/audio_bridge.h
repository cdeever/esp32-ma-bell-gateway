#ifndef __AUDIO_BRIDGE_H__
#define __AUDIO_BRIDGE_H__

#include "esp_err.h"
#include "freertos/ringbuf.h"

/**
 * @brief Initialize the audio bridge module
 *
 * Sets up I2S RX channel for receiving audio from PCM1808 ADC.
 * The TX channel is managed by audio_output module.
 * Must be called after audio_output_init().
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t audio_bridge_init(void);

/**
 * @brief Start audio bridging between I2S and Bluetooth
 *
 * Creates two FreeRTOS tasks:
 * - audio_rx_task: Reads audio from PCM1808 ADC (I2S RX) and sends to Bluetooth
 * - audio_tx_task: Reads audio from Bluetooth and writes to PCM5100 DAC (I2S TX)
 *
 * Should be called when Bluetooth audio connection is established.
 */
void audio_bridge_start(void);

/**
 * @brief Stop audio bridging
 *
 * Deletes the audio RX and TX tasks.
 * Should be called when Bluetooth audio connection is disconnected.
 */
void audio_bridge_stop(void);

/**
 * @brief Get the Bluetooth RX ring buffer handle
 *
 * Used by Bluetooth HFP incoming callback to write received audio from phone.
 * Audio flows: Bluetooth → BT RX ringbuf → audio_tx_task → I2S TX → Phone speaker
 *
 * @return Ring buffer handle for Bluetooth RX
 */
RingbufHandle_t audio_bridge_get_bt_rx_ringbuf(void);

/**
 * @brief Get the Bluetooth TX ring buffer handle
 *
 * Used by Bluetooth HFP outgoing callback to read audio to send to phone.
 * Audio flows: Phone mic → I2S RX → audio_rx_task → BT TX ringbuf → Bluetooth
 *
 * @return Ring buffer handle for Bluetooth TX
 */
RingbufHandle_t audio_bridge_get_bt_tx_ringbuf(void);

#endif /* __AUDIO_BRIDGE_H__ */
