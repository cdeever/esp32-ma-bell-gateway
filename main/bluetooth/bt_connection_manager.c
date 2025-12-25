/*
 * Bluetooth Connection Manager
 * Handles connection, reconnection, and pairing
 */

#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "bt_connection_manager.h"
#include "app/bluetooth/app_hf_msg_set.h"
#include "app/state/ma_bell_state.h"
#include "app/events/event_system.h"
#include "config/bluetooth_config.h"

static const char *TAG = "BT_CONN_MGR";

// Global variable to track if we're currently trying to connect
static bool is_connecting = false;

// Reconnection task handle
static TaskHandle_t reconnect_task_handle = NULL;

// Reconnection task
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
                    uint8_t inq_len = BT_DISCOVERY_DURATION;
                    uint8_t inq_num_rsps = BT_DISCOVERY_MAX_RESPONSES;
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

// GAP callback for connection management
void bt_connection_manager_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            ESP_LOGI(TAG, "Discovery state changed: %d", param->disc_st_chg.state);
            if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
                is_connecting = false;
                // Publish disconnection event if stopped while connecting
                event_publish(BT_EVENT_DISCONNECTED, NULL);
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
                        // Publish connection event (will be confirmed by HFP callback)
                        event_publish(BT_EVENT_CONNECTED, NULL);
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
            esp_bt_pin_code_t pin_code = BT_PIN_CODE;
            esp_bt_gap_pin_reply(param->pin_req.bda, true, BT_PIN_CODE_LEN, pin_code);
            break;

        default:
            break;
    }
}

// Initialize the Bluetooth connection manager
esp_err_t bt_connection_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Bluetooth connection manager");

    // Create reconnection task
    BaseType_t ret = xTaskCreate(bt_reconnect_task,
                                  BT_RECONNECT_TASK_NAME,
                                  BT_RECONNECT_TASK_STACK_SIZE,
                                  NULL,
                                  BT_RECONNECT_TASK_PRIORITY,
                                  &reconnect_task_handle);

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create reconnection task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Bluetooth connection manager initialized successfully");
    return ESP_OK;
}

// Get connection status
bool bt_connection_manager_is_connected(void)
{
    return ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED);
}
