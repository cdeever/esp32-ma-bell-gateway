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
#include "audio/audio_output.h"
#include "audio/audio_bridge.h"
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

    // Initialize audio output (I2S TX + tone generation)
    ESP_LOGI(TAG, "Initializing audio output...");
    ESP_ERROR_CHECK(audio_output_init());

    // Initialize audio bridge (I2S RX for phone mic)
    ESP_LOGI(TAG, "Initializing audio bridge...");
    ESP_ERROR_CHECK(audio_bridge_init());

    // Initialize communication subsystems
    // Note: WiFi initialized BEFORE Bluetooth to avoid coexistence issues during connection
    ESP_LOGI(TAG, "Initializing WiFi...");
    esp_err_t wifi_ret = wifi_init_and_connect();
    if (wifi_ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi initialization failed: %s", esp_err_to_name(wifi_ret));
        ESP_LOGE(TAG, "Device will continue without WiFi. See WIFI_SETUP.md for provisioning.");
    }

    // WiFi init is complete (success or timeout), now safe to start Bluetooth
    ESP_LOGI(TAG, "Initializing Bluetooth...");
    ESP_ERROR_CHECK(bluetooth_init());

    // Signal Bluetooth initialization complete
    ma_bell_state_update_network_bits(NET_STATE_BT_INIT_COMPLETE, 0);
    ESP_LOGI(TAG, "Bluetooth initialization complete");

    // Initialize application services
    ESP_LOGI(TAG, "Initializing web interface...");
    ESP_ERROR_CHECK(web_interface_init());

    // Signal web server initialization complete
    ma_bell_state_update_network_bits(NET_STATE_WEB_INIT_COMPLETE, 0);
    ESP_LOGI(TAG, "Web interface initialization complete");

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Ma Bell Gateway initialized successfully!");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "System Status:");
    ESP_LOGI(TAG, "  - WiFi: Connected (if credentials were valid)");
    ESP_LOGI(TAG, "  - Bluetooth: Ready for pairing");
    ESP_LOGI(TAG, "  - Web Interface: Active");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Waiting for Bluetooth device...");
}
