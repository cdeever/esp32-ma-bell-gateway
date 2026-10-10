#ifndef __EVENT_LOG_H__
#define __EVENT_LOG_H__

/**
 * @file event_log.h
 * @brief Key events of the gateway, queued for delivery to the tenant's log store
 *
 * This is not the serial log. It carries the few events worth keeping - the
 * gateway starting, the phone going off hook, a call starting - as one JSON
 * object per event. The MQTT subsystem takes them from the queue and publishes
 * them to the tenant's log topic; until it can, they wait here.
 */

#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

typedef enum {
    EVENT_LOG_INFO,
    EVENT_LOG_WARN,
    EVENT_LOG_ERROR,
} event_log_level_t;

/**
 * @brief Initialize the event log queue
 *
 * @return ESP_OK on success, ESP_ERR_NO_MEM on failure
 */
esp_err_t event_log_init(void);

/**
 * @brief Record an event
 *
 * Never blocks on delivery and is safe from any task (not from an ISR). When
 * the queue is full the oldest event is dropped, and the loss is reported in
 * the next event as a "dropped" field.
 *
 * @param level Severity
 * @param event Dotted event name, e.g. "phone.off_hook"; a field to filter on
 * @param extra_json Extra fields as a JSON fragment without braces, e.g.
 *                   "\"rssi\":-61", or NULL. Numbers here can be graphed.
 * @param fmt printf-style message for people
 */
void event_log_with(event_log_level_t level, const char *event, const char *extra_json,
                    const char *fmt, ...) __attribute__((format(printf, 4, 5)));

/** @brief Record an event with no extra fields */
#define event_log(level, event, ...) event_log_with((level), (event), NULL, __VA_ARGS__)

/**
 * @brief Escape text for use inside a JSON string, e.g. in extra_json
 *
 * Truncates to fit; dst is always terminated.
 */
void event_log_escape(const char *src, char *dst, size_t dst_len);

/**
 * @brief Take the oldest queued event, as a JSON object
 *
 * @param line Buffer for the event, at least EVENT_LOG_LINE_LEN bytes
 * @param line_len Size of the buffer
 * @param ticks_to_wait How long to wait for an event
 * @return true if an event was written to line
 */
bool event_log_receive(char *line, size_t line_len, TickType_t ticks_to_wait);

#endif /* __EVENT_LOG_H__ */
