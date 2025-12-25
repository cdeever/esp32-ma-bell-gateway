/*
 * Bluetooth Subsystem Initialization
 */

#include <string.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "bt_init.h"
#include "bt_app_core.h"
#include "bt_app_hf.h"
#include "bt_connection_manager.h"
#include "config/bluetooth_config.h"

static const char *TAG = "BT_INIT";

esp_err_t bluetooth_init(void)
{
    ESP_LOGI(TAG, "Initializing Bluetooth subsystem");

    // Step 1: Release BLE memory (we only use Classic BT)
    esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to release BLE memory: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 2: Initialize Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BT controller: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 3: Enable Bluetooth controller in Classic BT mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 4: Initialize Bluedroid stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 5: Enable Bluedroid stack
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 6: CRITICAL FIX - Start BT app task BEFORE registering callbacks
    // This creates the message queue that callbacks will use
    ESP_LOGI(TAG, "Starting BT application task (CRITICAL FIX)");
    bt_app_task_start_up();

    // Step 7: Set Bluetooth device name
    ret = esp_bt_gap_set_device_name(BT_DEVICE_NAME);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Set BT device name: %s", BT_DEVICE_NAME);

    // Step 8: Initialize HFP client profile
    ret = esp_hf_client_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HFP client: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 9: Set PIN code
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code = BT_PIN_CODE;
    ret = esp_bt_gap_set_pin(pin_type, BT_PIN_CODE_LEN, pin_code);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PIN code: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 10: Set discoverable and connectable mode
    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set scan mode: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 11: Register GAP callback for connection management
    ret = esp_bt_gap_register_callback(bt_connection_manager_gap_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GAP callback: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 12: Register HFP client callback
    ret = esp_hf_client_register_callback(bt_app_hf_client_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register HFP callback: %s", esp_err_to_name(ret));
        return ret;
    }

    // Step 13: Initialize connection manager (starts reconnection task)
    ret = bt_connection_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize connection manager: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Bluetooth subsystem initialized successfully");
    return ESP_OK;
}

esp_err_t bluetooth_shutdown(void)
{
    ESP_LOGI(TAG, "Shutting down Bluetooth subsystem");

    // Shutdown in reverse order
    bt_app_task_shut_down();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();

    ESP_LOGI(TAG, "Bluetooth subsystem shut down");
    return ESP_OK;
}
