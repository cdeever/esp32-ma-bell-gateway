/*
 * Simple Event System
 * Provides publish/subscribe mechanism for subsystem communication
 */

#include <string.h>
#include "event_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "EVENT_SYS";

#define MAX_SUBSCRIBERS 10

// Subscriber structure
typedef struct {
    event_type_t events;        // Bitmask of events this subscriber cares about
    event_callback_t callback;  // Callback function
    void* user_data;            // User data to pass to callback
    bool active;                // Whether this slot is active
} subscriber_t;

// Global subscriber list
static subscriber_t subscribers[MAX_SUBSCRIBERS];
static SemaphoreHandle_t subscribers_mutex = NULL;

esp_err_t event_system_init(void)
{
    ESP_LOGI(TAG, "Initializing event system");

    // Initialize subscriber list
    memset(subscribers, 0, sizeof(subscribers));

    // Create mutex for thread-safe access
    subscribers_mutex = xSemaphoreCreateMutex();
    if (subscribers_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Event system initialized successfully");
    return ESP_OK;
}

esp_err_t event_publish(event_type_t event, void* event_data)
{
    if (subscribers_mutex == NULL) {
        ESP_LOGW(TAG, "Event system not initialized");
        return ESP_FAIL;
    }

    // Take mutex to protect subscriber list
    if (xSemaphoreTake(subscribers_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to acquire mutex for event publish");
        return ESP_FAIL;
    }

    // Iterate through subscribers and call matching callbacks
    int callbacks_called = 0;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (subscribers[i].active && (subscribers[i].events & event)) {
            // This subscriber is interested in this event
            if (subscribers[i].callback != NULL) {
                subscribers[i].callback(event, subscribers[i].user_data);
                callbacks_called++;
            }
        }
    }

    xSemaphoreGive(subscribers_mutex);

    if (callbacks_called > 0) {
        ESP_LOGD(TAG, "Event 0x%x published to %d subscribers", event, callbacks_called);
    }

    return ESP_OK;
}

esp_err_t event_subscribe(event_type_t events,
                           event_callback_t callback,
                           void* user_data)
{
    if (subscribers_mutex == NULL) {
        ESP_LOGE(TAG, "Event system not initialized");
        return ESP_FAIL;
    }

    if (callback == NULL) {
        ESP_LOGE(TAG, "Callback cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Take mutex to protect subscriber list
    if (xSemaphoreTake(subscribers_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to acquire mutex for subscription");
        return ESP_FAIL;
    }

    // Find an empty slot
    esp_err_t ret = ESP_FAIL;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (!subscribers[i].active) {
            subscribers[i].events = events;
            subscribers[i].callback = callback;
            subscribers[i].user_data = user_data;
            subscribers[i].active = true;
            ESP_LOGI(TAG, "Subscriber registered for events 0x%x", events);
            ret = ESP_OK;
            break;
        }
    }

    xSemaphoreGive(subscribers_mutex);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register subscriber - max subscribers (%d) reached", MAX_SUBSCRIBERS);
    }

    return ret;
}
