#ifndef __TONES_H__
#define __TONES_H__

#include "config/audio_config.h"

/**
 * @brief North American telephone tone types
 *
 * Standard telephone signaling tones with proper frequencies and cadences.
 */
typedef enum {
    DIAL_TONE,           // 350 Hz + 440 Hz, continuous
    RINGBACK_TONE,       // 440 Hz + 480 Hz, 2s on / 4s off
    BUSY_SIGNAL,         // 480 Hz + 620 Hz, 0.5s on / 0.5s off
    REORDER_TONE,        // 480 Hz + 620 Hz, 0.25s on / 0.25s off (fast busy)
    OFF_HOOK_WARNING,    // 1400 Hz + 2060 Hz, 0.1s on / 0.1s off
    CONGESTION_TONE,     // 480 Hz + 620 Hz, 0.2s on / 0.3s off
    CONFIRMATION_TONE,   // 350 Hz + 440 Hz, 0.1s on / 0.1s off
    CALL_WAITING_TONE,   // 440 Hz, 0.3s on (single beep)
    SIT_TONE,            // 950 Hz + 1400 Hz, 0.274s (Special Information Tone)
    STUTTER_DIAL_TONE,   // 350 Hz + 440 Hz, 0.1s on / 0.1s off (voicemail indicator)
    NUM_TONES,
    TONE_NONE = NUM_TONES  // No tone playing
} tone_type_t;

/**
 * @brief Tone definition structure
 *
 * Defines the dual-frequency composition and cadence of a telephone tone.
 */
typedef struct {
    int freq1;           // Primary frequency in Hz
    int freq2;           // Secondary frequency in Hz (0 for single frequency)
    float duration_on;   // Duration of tone in seconds (-1 for continuous)
    float duration_off;  // Duration of silence in seconds (-1 for continuous)
} tone_t;

/**
 * @brief Get the tone definition for a given tone type
 *
 * @param tone The tone type
 * @return Pointer to tone definition, or NULL if invalid
 */
const tone_t *tone_get_definition(tone_type_t tone);

#endif /* __TONES_H__ */
