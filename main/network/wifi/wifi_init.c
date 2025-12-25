/*
 * WiFi Subsystem Initialization
 */

#include "wifi_init.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "WIFI_INIT";

// Internal event group for WiFi
static EventGroupHandle_t wifi_event_group = NULL;

esp_err_t wifi_init_and_connect(void)
{
    ESP_LOGI(TAG, "Initializing WiFi subsystem");

    // Create event group for WiFi events
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi event group");
        return ESP_FAIL;
    }

    // Initialize WiFi in station mode
    esp_err_t ret = wifi_init_sta(wifi_event_group);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    // Connect to WiFi using stored or default credentials
    ret = wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to connect to WiFi: %s (continuing without WiFi)", esp_err_to_name(ret));
        // Note: We don't return failure here - the system can work without WiFi
        return ESP_OK;
    }

    ESP_LOGI(TAG, "WiFi subsystem initialized successfully");
    return ESP_OK;
}
