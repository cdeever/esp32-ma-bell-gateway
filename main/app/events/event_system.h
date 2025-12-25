#ifndef __EVENT_SYSTEM_H__
#define __EVENT_SYSTEM_H__

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

// Event types (bitmask for efficient matching)
typedef enum {
    // Phone events
    PHONE_EVENT_OFF_HOOK           = (1 << 0),
    PHONE_EVENT_ON_HOOK            = (1 << 1),
    PHONE_EVENT_RINGING_START      = (1 << 2),
    PHONE_EVENT_RINGING_STOP       = (1 << 3),
    PHONE_EVENT_DIGIT_DIALED       = (1 << 4),

    // Bluetooth events
    BT_EVENT_CONNECTED             = (1 << 5),
    BT_EVENT_DISCONNECTED          = (1 << 6),
    BT_EVENT_AUDIO_CONNECTED       = (1 << 7),
    BT_EVENT_AUDIO_DISCONNECTED    = (1 << 8),
    BT_EVENT_CALL_STARTED          = (1 << 9),
    BT_EVENT_CALL_ENDED            = (1 << 10),

    // Network events
    WIFI_EVENT_CONNECTED_EV        = (1 << 11),
    WIFI_EVENT_DISCONNECTED_EV     = (1 << 12),
    WIFI_EVENT_IP_ACQUIRED_EV      = (1 << 13),

    // System events
    SYS_EVENT_ERROR                = (1 << 14),
    SYS_EVENT_LOW_BATTERY          = (1 << 15),
} event_type_t;

// Event callback function type
typedef void (*event_callback_t)(event_type_t event, void* user_data);

/**
 * @brief Initialize the event system
 *
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t event_system_init(void);

/**
 * @brief Publish an event to all subscribers
 *
 * @param event Event type to publish
 * @param event_data Optional event-specific data (can be NULL)
 * @return ESP_OK on success
 */
esp_err_t event_publish(event_type_t event, void* event_data);

/**
 * @brief Subscribe to specific events with a callback
 *
 * @param events Bitmask of events to subscribe to
 * @param callback Function to call when event occurs
 * @param user_data User data to pass to callback
 * @return ESP_OK on success, ESP_FAIL if max subscribers reached
 */
esp_err_t event_subscribe(event_type_t events,
                           event_callback_t callback,
                           void* user_data);

#endif /* __EVENT_SYSTEM_H__ */
