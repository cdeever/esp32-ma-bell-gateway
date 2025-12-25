#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#include "driver/i2s_std.h"

// I2S configuration
#define AUDIO_I2S_PORT              I2S_NUM_0
#define AUDIO_SAMPLE_RATE           8000
#define AUDIO_BUFFER_SIZE           1024
#define AUDIO_PLAY_DURATION         5  // seconds

// HFP Audio (if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI)
#define AUDIO_HFP_RINGBUF_SIZE      3600

#endif /* __AUDIO_CONFIG_H__ */
