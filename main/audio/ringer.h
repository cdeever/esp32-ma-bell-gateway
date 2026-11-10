#ifndef __RINGER_H__
#define __RINGER_H__

#include "esp_err.h"

/**
 * @brief Ring on the speaker while the cell phone has an incoming call
 *
 * Plays RING_TONE (an electronic ringer warble, 2s on / 4s off) on the I2S
 * output from PHONE_EVENT_RINGING_START until PHONE_EVENT_RINGING_STOP: the
 * call is answered, declined or missed, or the phone disconnects. A stop
 * leaves any other tone alone, such as one started by a tone test code.
 *
 * Enabled by CONFIG_MA_BELL_SPEAKER_RING; does nothing when it is off.
 * Call after audio_output_init() and event_system_init().
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ringer_init(void);

#endif /* __RINGER_H__ */
