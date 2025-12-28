#ifndef __AUDIO_OUTPUT_H__
#define __AUDIO_OUTPUT_H__

#include "esp_err.h"
#include "driver/i2s_std.h"
#include "audio/tones.h"

/**
 * @brief Initialize the audio I/O subsystem
 *
 * Sets up I2S TX and RX channels for bidirectional audio:
 * - TX: Audio output to phone speaker (tones and BT audio)
 * - RX: Audio input from phone microphone (to Bluetooth)
 *
 * Creates the tone generation task.
 * Must be called before audio_bridge_init().
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t audio_output_init(void);

/**
 * @brief Get the I2S RX channel handle
 *
 * Used by audio_bridge to read audio from the phone microphone.
 * Must only be called after audio_output_init().
 *
 * @return I2S RX channel handle, or NULL if not initialized
 */
i2s_chan_handle_t audio_output_get_rx_handle(void);

/**
 * @brief Write raw PCM audio data to output
 *
 * Used by audio_bridge for Bluetooth audio passthrough.
 * If a tone is currently playing, the audio may be blocked or mixed.
 *
 * @param data Pointer to 16-bit PCM samples
 * @param len Number of bytes to write
 * @param bytes_written Pointer to store actual bytes written
 * @param timeout_ms Maximum time to wait
 * @return ESP_OK on success
 */
esp_err_t audio_output_write(const void *data, size_t len, size_t *bytes_written, uint32_t timeout_ms);

/**
 * @brief Start playing a telephone tone
 *
 * Thread-safe. Interrupts any Bluetooth audio passthrough.
 * Tone plays until stopped or another tone is started.
 *
 * @param tone The tone type to play (DIAL_TONE, BUSY_SIGNAL, etc.)
 * @return ESP_OK on success
 */
esp_err_t audio_output_play_tone(tone_type_t tone);

/**
 * @brief Stop the currently playing tone
 *
 * Thread-safe. Allows Bluetooth audio passthrough to resume.
 *
 * @return ESP_OK on success
 */
esp_err_t audio_output_stop_tone(void);

/**
 * @brief Check if a tone is currently playing
 *
 * @return true if a tone is active, false otherwise
 */
bool audio_output_tone_active(void);

/**
 * @brief Get the current tone being played
 *
 * @return Current tone type, or TONE_NONE if no tone is playing
 */
tone_type_t audio_output_get_current_tone(void);

#endif /* __AUDIO_OUTPUT_H__ */
