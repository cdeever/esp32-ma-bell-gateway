/*
 * Event Log
 * Queues the gateway's key events as JSON lines for the MQTT subsystem to deliver
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "event_log.h"
#include "config/system_config.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "EVENT_LOG";

static QueueHandle_t s_queue = NULL;
static SemaphoreHandle_t s_mutex = NULL;
static uint32_t s_dropped = 0;

// Formatting buffers, guarded by s_mutex so callers need no stack for them
static char s_msg[EVENT_LOG_LINE_LEN];
static char s_escaped[EVENT_LOG_LINE_LEN];
static char s_line[EVENT_LOG_LINE_LEN];

static const char *level_name(event_log_level_t level)
{
    switch (level) {
        case EVENT_LOG_WARN:  return "warn";
        case EVENT_LOG_ERROR: return "error";
        default:              return "info";
    }
}

void event_log_escape(const char *src, char *dst, size_t dst_len)
{
    size_t o = 0;
    if (dst == NULL || dst_len == 0) {
        return;
    }
    if (src == NULL) {
        src = "";
    }
    for (; *src != '\0' && o + 7 < dst_len; src++) {
        unsigned char c = (unsigned char)*src;
        if (c == '"' || c == '\\') {
            dst[o++] = '\\';
            dst[o++] = (char)c;
        } else if (c < 0x20) {
            o += snprintf(dst + o, dst_len - o, "\\u%04x", c);
        } else {
            dst[o++] = (char)c;
        }
    }

    // A name cut short can end partway through a UTF-8 character, and that
    // would stop the whole line being read as JSON: drop the partial one
    size_t lead = o;
    while (lead > 0 && ((unsigned char)dst[lead - 1] & 0xC0) == 0x80) {
        lead--;
    }
    if (lead > 0 && ((unsigned char)dst[lead - 1] & 0x80)) {
        unsigned char first = (unsigned char)dst[lead - 1];
        size_t need = (first & 0xE0) == 0xC0 ? 2 : (first & 0xF0) == 0xE0 ? 3 : (first & 0xF8) == 0xF0 ? 4 : 0;
        if (need == 0 || o - (lead - 1) < need) {
            o = lead - 1;
        }
    }
    dst[o] = '\0';
}

esp_err_t event_log_init(void)
{
    s_queue = xQueueCreate(EVENT_LOG_QUEUE_LEN, EVENT_LOG_LINE_LEN);
    s_mutex = xSemaphoreCreateMutex();
    if (s_queue == NULL || s_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create event log queue");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Event log initialized (%d events of up to %d bytes)", EVENT_LOG_QUEUE_LEN, EVENT_LOG_LINE_LEN);
    return ESP_OK;
}

void event_log_with(event_log_level_t level, const char *event, const char *extra_json, const char *fmt, ...)
{
    if (s_queue == NULL || s_mutex == NULL || event == NULL || fmt == NULL) {
        return;
    }
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(s_msg, sizeof(s_msg), fmt, args);
    va_end(args);
    event_log_escape(s_msg, s_escaped, sizeof(s_escaped) / 2);

    // No clock on the gateway: the broker's receive time dates the event, and
    // uptime_ms orders events that were queued while it was offline
    int len = snprintf(s_line, sizeof(s_line),
                       "{\"level\":\"%s\",\"event\":\"%s\",\"msg\":\"%s\",\"uptime_ms\":%lld",
                       level_name(level), event, s_escaped, (long long)(esp_timer_get_time() / 1000));
    if (len > 0 && len < (int)sizeof(s_line) && s_dropped > 0) {
        len += snprintf(s_line + len, sizeof(s_line) - len, ",\"dropped\":%lu", (unsigned long)s_dropped);
    }
    if (len > 0 && len < (int)sizeof(s_line) && extra_json != NULL && extra_json[0] != '\0') {
        len += snprintf(s_line + len, sizeof(s_line) - len, ",%s", extra_json);
    }
    if (len > 0 && len < (int)sizeof(s_line)) {
        len += snprintf(s_line + len, sizeof(s_line) - len, "}");
    }

    if (len <= 0 || len >= (int)sizeof(s_line)) {
        ESP_LOGW(TAG, "Event '%s' does not fit in %d bytes, dropped", event, EVENT_LOG_LINE_LEN);
        s_dropped++;
        xSemaphoreGive(s_mutex);
        return;
    }

    if (xQueueSend(s_queue, s_line, 0) != pdTRUE) {
        // Full: make room by dropping the oldest, which is the one least
        // likely to explain what is happening now
        char *oldest = s_msg;
        if (xQueueReceive(s_queue, oldest, 0) == pdTRUE) {
            s_dropped++;
        }
        if (xQueueSend(s_queue, s_line, 0) != pdTRUE) {
            s_dropped++;
        }
    } else {
        s_dropped = 0;
    }

    xSemaphoreGive(s_mutex);
}

bool event_log_receive(char *line, size_t line_len, TickType_t ticks_to_wait)
{
    if (s_queue == NULL || line == NULL || line_len < EVENT_LOG_LINE_LEN) {
        return false;
    }
    return xQueueReceive(s_queue, line, ticks_to_wait) == pdTRUE;
}
