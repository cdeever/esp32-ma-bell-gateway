#include "ringer.h"
#include "audio_output.h"
#include "app/events/event_system.h"
#include "app/state/ma_bell_state.h"
#include "sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "ringer";

#if CONFIG_MA_BELL_SPEAKER_RING

// Runs in the publisher's context (the Bluetooth task); only sets the tone
static void ringer_on_event(event_type_t event, void *user_data)
{
    (void)user_data;
    bool ringing = ma_bell_state_phone_bits_set(PHONE_STATE_RINGING);
    bool off_hook = ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK);

    if (event & (PHONE_EVENT_RINGING_START | PHONE_EVENT_ON_HOOK)) {
        // A line rings only while the handset is down
        if (ringing && !off_hook) {
            ESP_LOGI(TAG, "Incoming call: ringing the speaker");
            audio_output_play_tone(RING_TONE);
        } else if (event & PHONE_EVENT_RINGING_START) {
            ESP_LOGI(TAG, "Incoming call while off-hook: not ringing");
        }
    } else if (event & (PHONE_EVENT_RINGING_STOP | PHONE_EVENT_OFF_HOOK)) {
        // Stopped by the call ending, or by lifting the handset to answer
        if (audio_output_get_current_tone() == RING_TONE) {
            ESP_LOGI(TAG, "Ringing stopped");
            audio_output_stop_tone();
        }
    }
}

esp_err_t ringer_init(void)
{
    esp_err_t ret = event_subscribe(PHONE_EVENT_RINGING_START | PHONE_EVENT_RINGING_STOP |
                                    PHONE_EVENT_OFF_HOOK | PHONE_EVENT_ON_HOOK,
                                    ringer_on_event, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to subscribe to ringing events: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Speaker ring enabled");
    return ESP_OK;
}

#else

esp_err_t ringer_init(void)
{
    (void)TAG;
    return ESP_OK;
}

#endif /* CONFIG_MA_BELL_SPEAKER_RING */
