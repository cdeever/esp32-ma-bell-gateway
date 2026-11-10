#include "line.h"
#include "config/line_config.h"
#include "app/events/event_system.h"
#include "app/state/ma_bell_state.h"
#include "audio/audio_output.h"
#include "esp_hf_client_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "line";

typedef enum {
    LINE_ON_HOOK,      // Handset down
    LINE_DEAD,         // Off-hook with no cell phone connected: silence
    LINE_DIAL_TONE,    // Off-hook, waiting for a number
    LINE_REORDER,      // Off-hook too long without dialing
    LINE_HOWLER,       // Off-hook warning, until the handset is replaced
    LINE_ANSWERING,    // Lifted while ringing; waiting for the call to start
    LINE_IN_CALL,      // The handset is in a call
    LINE_CALL_ENDED,   // The far end hung up; silence before dial tone
} line_stage_t;

static const char *stage_names[] = {
    "on-hook", "dead", "dial tone", "reorder", "howler", "answering", "in call", "call ended",
};

static QueueHandle_t line_queue = NULL;
static line_stage_t stage = LINE_ON_HOOK;
static TickType_t deadline = 0;    // When the current stage times out
static bool has_deadline = false;

static void set_stage(line_stage_t next, tone_type_t tone, uint32_t timeout_ms)
{
    if (next != stage) {
        ESP_LOGI(TAG, "%s -> %s", stage_names[stage], stage_names[next]);
    }
    stage = next;

    // Only replace a tone this module would play; leave the ring and test tones alone
    tone_type_t current = audio_output_get_current_tone();
    if (tone != TONE_NONE) {
        audio_output_play_tone(tone);
    } else if (current == DIAL_TONE || current == REORDER_TONE || current == OFF_HOOK_WARNING) {
        audio_output_stop_tone();
    }

    has_deadline = timeout_ms > 0;
    deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);
}

// Off-hook with no call: dial tone if a cell phone is there to dial on
static void offer_dial_tone(void)
{
    if (ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED)) {
        set_stage(LINE_DIAL_TONE, DIAL_TONE, LINE_DIAL_TONE_TIMEOUT_MS);
    } else {
        ESP_LOGI(TAG, "No cell phone connected: the line is dead");
        set_stage(LINE_DEAD, TONE_NONE, 0);
    }
}

static void handle_off_hook(void)
{
    if (ma_bell_state_phone_bits_set(PHONE_STATE_RINGING)) {
        ESP_LOGI(TAG, "Answering the call");
        set_stage(LINE_ANSWERING, TONE_NONE, 0);
        esp_hf_client_answer_call();
    } else if (ma_bell_state_bluetooth_bits_set(BT_STATE_IN_CALL)) {
        set_stage(LINE_IN_CALL, TONE_NONE, 0);
    } else {
        offer_dial_tone();
    }
}

static void handle_on_hook(void)
{
    if (stage == LINE_IN_CALL || stage == LINE_ANSWERING) {
        ESP_LOGI(TAG, "Hanging up the call");
        esp_hf_client_reject_call();  // AT+CHUP ends the active call
    }
    set_stage(LINE_ON_HOOK, TONE_NONE, 0);
}

static void handle_event(event_type_t event)
{
    switch (event) {
    case PHONE_EVENT_OFF_HOOK:
        handle_off_hook();
        break;

    case PHONE_EVENT_ON_HOOK:
        handle_on_hook();
        break;

    case BT_EVENT_CALL_STARTED:
        // Answered here, or placed or answered on the cell phone while the handset is up
        if (stage != LINE_ON_HOOK) {
            set_stage(LINE_IN_CALL, TONE_NONE, 0);
        }
        break;

    case BT_EVENT_CALL_ENDED:
        if (stage == LINE_IN_CALL || stage == LINE_ANSWERING) {
            set_stage(LINE_CALL_ENDED, TONE_NONE, LINE_CALL_ENDED_SILENCE_MS);
        }
        break;

    case PHONE_EVENT_RINGING_STOP:
        // The caller gave up before the answer reached the cell phone
        if (stage == LINE_ANSWERING && !ma_bell_state_bluetooth_bits_set(BT_STATE_IN_CALL)) {
            ESP_LOGI(TAG, "The call ended before it was answered");
            set_stage(LINE_CALL_ENDED, TONE_NONE, LINE_CALL_ENDED_SILENCE_MS);
        }
        break;

    case BT_EVENT_CONNECTED:
        if (stage == LINE_DEAD) {
            offer_dial_tone();
        }
        break;

    case BT_EVENT_DISCONNECTED:
        if (stage != LINE_ON_HOOK) {
            ESP_LOGI(TAG, "Cell phone disconnected: the line is dead");
            set_stage(LINE_DEAD, TONE_NONE, 0);
        }
        break;

    default:
        break;
    }
}

static void handle_timeout(void)
{
    switch (stage) {
    case LINE_DIAL_TONE:
        set_stage(LINE_REORDER, REORDER_TONE, LINE_REORDER_TIMEOUT_MS);
        break;
    case LINE_REORDER:
        set_stage(LINE_HOWLER, OFF_HOOK_WARNING, 0);
        break;
    case LINE_CALL_ENDED:
        offer_dial_tone();
        break;
    default:
        has_deadline = false;
        break;
    }
}

static void line_task(void *arg)
{
    // The handset may already be off the hook at boot
    if (ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK)) {
        handle_off_hook();
    }

    while (1) {
        TickType_t wait = portMAX_DELAY;
        if (has_deadline) {
            TickType_t now = xTaskGetTickCount();
            wait = (int32_t)(deadline - now) > 0 ? deadline - now : 0;
        }

        event_type_t event;
        if (xQueueReceive(line_queue, &event, wait) == pdTRUE) {
            handle_event(event);
        } else {
            handle_timeout();
        }
    }
}

// Runs in the publisher's context: hand the event to the line task
static void line_on_event(event_type_t event, void *user_data)
{
    (void)user_data;
    if (xQueueSend(line_queue, &event, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Line event queue full, dropped event 0x%x", event);
    }
}

esp_err_t line_init(void)
{
    line_queue = xQueueCreate(LINE_EVENT_QUEUE_LEN, sizeof(event_type_t));
    if (line_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create line event queue");
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = event_subscribe(PHONE_EVENT_OFF_HOOK | PHONE_EVENT_ON_HOOK |
                                    PHONE_EVENT_RINGING_STOP |
                                    BT_EVENT_CALL_STARTED | BT_EVENT_CALL_ENDED |
                                    BT_EVENT_CONNECTED | BT_EVENT_DISCONNECTED,
                                    line_on_event, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to subscribe to line events: %s", esp_err_to_name(ret));
        return ret;
    }

    if (xTaskCreate(line_task, "line", LINE_TASK_STACK_SIZE, NULL, LINE_TASK_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create line task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Line ready");
    return ESP_OK;
}
