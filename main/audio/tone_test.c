#include "tone_test.h"
#include "audio_output.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "tone_test";

#if CONFIG_MA_BELL_TONE_TEST

// How long each tone plays in the *9 sequence
#define TONE_TEST_STEP_US (8 * 1000 * 1000)

typedef struct {
    tone_type_t tone;
    const char *name;
} tone_test_entry_t;

// Index = the digit dialed after '*'. TONE_NONE marks an unused digit.
static const tone_test_entry_t codes[10] = {
    [0] = { TONE_NONE,         "stop" },
    [1] = { DIAL_TONE,         "dial tone" },
    [2] = { RINGBACK_TONE,     "ringback" },
    [3] = { BUSY_SIGNAL,       "busy signal" },
    [4] = { REORDER_TONE,      "reorder (fast busy)" },
    [5] = { OFF_HOOK_WARNING,  "off-hook warning" },
    [6] = { STUTTER_DIAL_TONE, "stutter dial tone" },
    [7] = { TONE_NONE,         "quieter" },
    [8] = { TONE_NONE,         "louder" },
    [9] = { TONE_NONE,         "sequence" },
};

// The *9 sequence: the codes above, in order
static const int sequence[] = { 1, 2, 3, 4, 5, 6 };
#define SEQUENCE_LEN (sizeof(sequence) / sizeof(sequence[0]))

static esp_timer_handle_t sequence_timer = NULL;
static int sequence_step = 0;

static void play_code(int digit)
{
    ESP_LOGI(TAG, "*%d: %s", digit, codes[digit].name);
    audio_output_play_tone(codes[digit].tone);
}

static void sequence_timer_cb(void *arg)
{
    if (sequence_step >= (int)SEQUENCE_LEN) {
        ESP_LOGI(TAG, "Sequence finished");
        audio_output_stop_tone();
        return;
    }
    play_code(sequence[sequence_step++]);
    esp_timer_start_once(sequence_timer, TONE_TEST_STEP_US);
}

bool tone_test_handle_dialed(const char *number)
{
    // A test code is exactly '*' and one digit
    if (number == NULL || number[0] != '*' || number[1] < '0' || number[1] > '9' ||
        number[2] != '\0') {
        return false;
    }
    int digit = number[1] - '0';
    if (codes[digit].name == NULL) {
        return false;
    }

    if (sequence_timer == NULL) {
        const esp_timer_create_args_t args = {
            .callback = sequence_timer_cb,
            .name = "tone_test_seq",
        };
        if (esp_timer_create(&args, &sequence_timer) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create sequence timer");
            return true;
        }
    }

    if (digit == 7 || digit == 8) {
        // Level change: the tone that is playing keeps playing
        ESP_LOGI(TAG, "*%d: %s", digit, codes[digit].name);
        audio_output_tone_level_step(digit == 8 ? 1 : -1);
        return true;
    }

    // Any new tone code ends a running sequence
    esp_timer_stop(sequence_timer);

    if (digit == 9) {
        ESP_LOGI(TAG, "*9: playing each tone for %d s", TONE_TEST_STEP_US / 1000000);
        sequence_step = 0;
        sequence_timer_cb(NULL);
    } else {
        play_code(digit);
    }
    return true;
}

#else

bool tone_test_handle_dialed(const char *number)
{
    (void)number;
    (void)TAG;
    return false;
}

#endif /* CONFIG_MA_BELL_TONE_TEST */
