
#ifndef __TONES_H__
#define __TONES_H__

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 8000
#define BUFFER_SIZE 1024

// Define tone constants
typedef enum {
    DIAL_TONE,
    RINGBACK_TONE,
    BUSY_SIGNAL,
    REORDER_TONE,
    OFF_HOOK_WARNING,
    CONGESTION_TONE,
    CONFIRMATION_TONE,
    CALL_WAITING_TONE,
    SIT_TONE,
    STUTTER_DIAL_TONE,
    NUM_TONES
} tone_type_t;

extern volatile tone_type_t current_tone;

// Tone structure
typedef struct {
    int freq1;
    int freq2;
    float duration_on;
    float duration_off;
} tone_t;


void i2s_init();
void generate_tone_task(void *param);

#endif