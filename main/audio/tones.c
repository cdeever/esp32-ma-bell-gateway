#include "tones.h"
#include <stddef.h>

/**
 * @brief North American telephone tone definitions
 *
 * Each tone is defined with dual frequencies and on/off cadence.
 * Frequencies are in Hz, durations in seconds (-1 = continuous).
 */
static const tone_t tones[NUM_TONES] = {
    // DIAL_TONE: 350 Hz + 440 Hz, continuous
    {350, 440, -1, -1},

    // RINGBACK_TONE: 440 Hz + 480 Hz, 2s on / 4s off
    {440, 480, 2.0f, 4.0f},

    // BUSY_SIGNAL: 480 Hz + 620 Hz, 0.5s on / 0.5s off
    {480, 620, 0.5f, 0.5f},

    // REORDER_TONE (Fast Busy): 480 Hz + 620 Hz, 0.25s on / 0.25s off
    {480, 620, 0.25f, 0.25f},

    // OFF_HOOK_WARNING: 1400 Hz + 2060 Hz + 2450 Hz + 2600 Hz alternating
    // Simplified to 1400 Hz + 2060 Hz, 0.1s on / 0.1s off
    {1400, 2060, 0.1f, 0.1f},

    // CONGESTION_TONE: 480 Hz + 620 Hz, 0.2s on / 0.3s off
    {480, 620, 0.2f, 0.3f},

    // CONFIRMATION_TONE: 350 Hz + 440 Hz, short beeps
    {350, 440, 0.1f, 0.1f},

    // CALL_WAITING_TONE: 440 Hz single beep, 0.3s
    {440, 0, 0.3f, -1},

    // SIT_TONE (Special Information Tone): First segment
    // Full SIT is 950 Hz, 1400 Hz, 1800 Hz sequence - this is simplified
    {950, 1400, 0.274f, 0.0f},

    // STUTTER_DIAL_TONE: 350 Hz + 440 Hz, rapid stutter (voicemail waiting)
    {350, 440, 0.1f, 0.1f}
};

const tone_t *tone_get_definition(tone_type_t tone)
{
    if (tone >= NUM_TONES) {
        return NULL;
    }
    return &tones[tone];
}
