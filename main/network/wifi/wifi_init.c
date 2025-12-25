/*
 * WiFi Subsystem Initialization
 */

#include "wifi_init.h"
#include "wifi.h"
#include "config/wifi_config.h"
#include "app/state/ma_bell_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"

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

    // Initialize WiFi in station mode (config is set inside this function)
    esp_err_t ret = wifi_init_sta(wifi_event_group);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to initialize WiFi: %s (continuing without WiFi)", esp_err_to_name(ret));
        // Note: We don't return failure here - the system can work without WiFi
        return ESP_OK;
    }

    // Wait for WiFi to actually connect or fail before returning
    ESP_LOGI(TAG, "Waiting for WiFi connection to complete...");
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,  // Don't clear bits
        pdFALSE,  // Wait for either bit (OR)
        portMAX_DELAY  // Wait forever until connection succeeds or all retries fail
    );

    // Set initialization complete state regardless of connection outcome
    ma_bell_state_update_network_bits(NET_STATE_WIFI_INIT_COMPLETE, 0);
    ESP_LOGI(TAG, "WiFi initialization complete, signaling dependent subsystems");

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to AP");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}
