/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_console.h"
#include "bluetooth/bt_app_core.h"
#include "bluetooth/bt_app_hf.h"
#include "app/bluetooth/app_hf_msg_set.h"
#include "network/wifi.h"
#include "storage/storage.h"
#include "app/tones.h"
#include "network/web/web_interface.h"
#include "hardware/gpio_pcm_config.h"
#include "app/state/ma_bell_state.h"

static const char *TAG = "MAIN";

#define BT_RECONNECT_INTERVAL_MS 10000  // 10 seconds

// Global variable to track if we're currently trying to connect
static bool is_connecting = false;

// Callback for Bluetooth GAP events
static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            ESP_LOGI(TAG, "Discovery state changed: %d", param->disc_st_chg.state);
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                is_connecting = false;
            }
            break;

        case ESP_BT_GAP_DISC_RES_EVT:
            if (is_connecting) {
                esp_bd_addr_t last_paired_addr;
                char last_paired_name[32];
                esp_err_t ret = app_hf_get_paired_device(last_paired_addr, last_paired_name, sizeof(last_paired_name));
                if (ret == ESP_OK) {
                    // Check if this is our paired device
                    if (memcmp(param->disc_res.bda, last_paired_addr, ESP_BD_ADDR_LEN) == 0) {
                        ESP_LOGI(TAG, "Found paired device, attempting to connect");
                        esp_bt_gap_cancel_discovery();
                        esp_hf_client_connect(param->disc_res.bda);
                    }
                }
            }
            break;

        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Authentication success with device: %s", param->auth_cmpl.device_name);
                // Convert device name to string and store paired device info
                char device_name[32] = {0};
                strncpy(device_name, (const char*)param->auth_cmpl.device_name, sizeof(device_name) - 1);
                app_hf_store_paired_device(param->auth_cmpl.bda, device_name);
                ESP_LOGI(TAG, "Stored paired device: %s", device_name);
            } else {
                ESP_LOGE(TAG, "Authentication failed: %d", param->auth_cmpl.stat);
                is_connecting = false;
            }
            break;

        case ESP_BT_GAP_PIN_REQ_EVT:
            ESP_LOGI(TAG, "PIN request from device: %02x:%02x:%02x:%02x:%02x:%02x",
                     param->pin_req.bda[0], param->pin_req.bda[1], param->pin_req.bda[2],
                     param->pin_req.bda[3], param->pin_req.bda[4], param->pin_req.bda[5]);
            esp_bt_pin_code_t pin_code = {'0', '0', '0', '0'};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            break;

        default:
            break;
    }
}

static void bt_reconnect_task(void *pvParameters)
{
    while (1) {
        // Only attempt reconnection if we're not already connected and not currently trying to connect
        if (!ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED) && !is_connecting) {
            esp_bd_addr_t last_paired_addr;
            char last_paired_name[32];
            esp_err_t ret = app_hf_get_paired_device(last_paired_addr, last_paired_name, sizeof(last_paired_name));
            
            if (ret == ESP_OK) {
                // Check if we have a valid stored device (not all zeros)
                bool has_valid_device = false;
                for (int i = 0; i < ESP_BD_ADDR_LEN; i++) {
                    if (last_paired_addr[i] != 0) {
                        has_valid_device = true;
                        break;
                    }
                }
                
                if (has_valid_device) {
                    ESP_LOGI(TAG, "Found stored device %s, attempting to reconnect", last_paired_name);
                    is_connecting = true;
                    
                    // Start inquiry to find the device
                    esp_bt_inq_mode_t inq_mode = ESP_BT_INQ_MODE_GENERAL_INQUIRY;
                    uint8_t inq_len = 10;  // 10 * 1.28s = 12.8s inquiry time
                    uint8_t inq_num_rsps = 0;  // Unlimited responses
                    ret = esp_bt_gap_start_discovery(inq_mode, inq_len, inq_num_rsps);
                    if (ret != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to start discovery: %s", esp_err_to_name(ret));
                        is_connecting = false;
                    }
                } else {
                    ESP_LOGI(TAG, "No paired device found, waiting for new connection");
                    // Set discoverable and connectable mode for new connections
                    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));
                }
            } else {
                ESP_LOGI(TAG, "No paired device found, waiting for new connection");
                // Set discoverable and connectable mode for new connections
                ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(BT_RECONNECT_INTERVAL_MS));
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Bluetooth
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // Initialize state management
    ma_bell_state_init();

    // Initialize GPIO and PCM
    app_gpio_pcm_io_cfg();

    // Initialize WiFi
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(wifi_init_sta(wifi_event_group));
    ESP_ERROR_CHECK(wifi_connect());

    // Initialize web server
    web_interface_init();

    // Set Bluetooth device name
    ESP_ERROR_CHECK(esp_bt_gap_set_device_name("MA BELL"));

    // Initialize HFP client profile
    ESP_ERROR_CHECK(esp_hf_client_init());

    // Set PIN code to '0000'
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code = {'0', '0', '0', '0'};
    ESP_ERROR_CHECK(esp_bt_gap_set_pin(pin_type, 4, pin_code));

    // Set discoverable and connectable mode
    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    // Register Bluetooth callbacks
    ESP_ERROR_CHECK(esp_bt_gap_register_callback(esp_bt_gap_cb));
    ESP_ERROR_CHECK(esp_hf_client_register_callback(bt_app_hf_client_cb));

    // Create Bluetooth reconnection task
    xTaskCreate(bt_reconnect_task, "bt_reconnect", 4096, NULL, 5, NULL);
}
