/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_log.h"
#include "nvs_flash.h"

// Configuration
#include "config/system_config.h"

// Subsystem initialization
#include "storage/storage.h"
#include "app/state/ma_bell_state.h"
#include "hardware/hardware_init.h"
#include "bluetooth/bt_init.h"
#include "network/wifi/wifi_init.h"
#include "app/web/web_interface.h"
#include "app/events/event_system.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Ma Bell Gateway starting...");

    // Initialize NVS (required by storage, WiFi, Bluetooth)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "NVS partition needs erase, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    // Initialize core subsystems
    ESP_LOGI(TAG, "Initializing core subsystems...");
    ESP_ERROR_CHECK(event_system_init());
    ESP_ERROR_CHECK(ma_bell_state_init());
    ESP_ERROR_CHECK(storage_init());

    // Initialize hardware peripherals
    ESP_LOGI(TAG, "Initializing hardware...");
    ESP_ERROR_CHECK(hardware_init());

    // Initialize communication subsystems
    ESP_LOGI(TAG, "Initializing Bluetooth...");
    ESP_ERROR_CHECK(bluetooth_init());  // CRITICAL FIX: bt_app_task_start_up() called here

    ESP_LOGI(TAG, "Initializing WiFi...");
    ESP_ERROR_CHECK(wifi_init_and_connect());

    // Initialize application services
    ESP_LOGI(TAG, "Initializing web interface...");
    ESP_ERROR_CHECK(web_interface_init());

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Ma Bell Gateway initialized successfully!");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "System Status:");
    ESP_LOGI(TAG, "  - Bluetooth: Ready for pairing");
    ESP_LOGI(TAG, "  - WiFi: Attempting connection");
    ESP_LOGI(TAG, "  - Web Interface: Active");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Waiting for Bluetooth device...");
}
