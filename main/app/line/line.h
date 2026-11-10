#ifndef __LINE_H__
#define __LINE_H__

#include "esp_err.h"

/**
 * @brief The central office's side of the phone line
 *
 * Reacts to the hook switch the way a central office would:
 *
 * - Lifted while ringing: answers the call on the cell phone.
 * - Lifted with no call: dial tone; after LINE_DIAL_TONE_TIMEOUT_MS with
 *   nothing dialed, reorder; after LINE_REORDER_TIMEOUT_MS more, the off-hook
 *   warning until the handset is replaced. With no cell phone connected the
 *   line is dead (silent), and dial tone starts when the phone connects.
 * - Lifted while the cell phone is already in a call: joins it.
 * - Replaced during a call the handset is in: hangs the call up.
 * - The far end hangs up: silence for LINE_CALL_ENDED_SILENCE_MS, then dial
 *   tone again.
 *
 * Events are handled on the line's own task, in order.
 * Call after event_system_init(), ma_bell_state_init(), hardware_init() and
 * audio_output_init().
 *
 * @return ESP_OK on success, error code on failure
 */
esp_err_t line_init(void);

#endif /* __LINE_H__ */
