/*
 * WiFi Subsystem Initialization
 */

#include "wifi_init.h"
#include "wifi.h"
#include "config/wifi_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

    // Allow WiFi and BT coexistence to stabilize before attempting connection
    ESP_LOGI(TAG, "Waiting for WiFi to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(500));

    // Connect to WiFi using stored or default credentials
    ret = wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to connect to WiFi: %s (continuing without WiFi)", esp_err_to_name(ret));
        // Note: We don't return failure here - the system can work without WiFi
        return ESP_OK;
    }

    // CRITICAL: Wait for WiFi to actually connect or fail before returning
    ESP_LOGI(TAG, "Waiting for WiFi connection to complete...");
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,  // Don't clear bits
        pdFALSE,  // Wait for either bit (OR)
        pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT * 1000)  // 30 second timeout
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "WiFi connection failed, continuing without WiFi");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "WiFi connection timed out, continuing without WiFi");
        return ESP_OK;
    }
}
