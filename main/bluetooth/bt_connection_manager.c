/*
 * Bluetooth Connection Manager
 * Handles connection, reconnection, and pairing
 */

#include <stdio.h>
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
#include "app/events/event_log.h"
#include "config/bluetooth_config.h"

static const char *TAG = "BT_CONN_MGR";

// Set while a connection attempt to the paired phone is outstanding
static bool is_connecting = false;

// The radio link to a phone is up, whether or not the hands-free session is
static bool acl_up = false;

// A hands-free session was established on the current radio link
static bool session_was_up = false;

// The phone's user disconnected the gateway. It is not reconnected until the
// phone connects again, the phone goes away and comes back, or the gateway
// restarts.
static bool held_off = false;

// When the hands-free session last closed
static TickType_t session_closed_at = 0;

// Reconnection task handle
static TaskHandle_t reconnect_task_handle = NULL;

// Cache for paired device info (avoid repeated NVS queries)
static struct {
    bool valid;
    esp_bd_addr_t addr;
    char name[32];
} paired_device_cache = {.valid = false};

// Load the paired phone from storage, if it is not already cached
static bool bt_load_paired_device(void)
{
    if (paired_device_cache.valid) {
        return true;
    }

    esp_err_t ret = app_hf_get_paired_device(
        paired_device_cache.addr,
        paired_device_cache.name,
        sizeof(paired_device_cache.name)
    );
    if (ret != ESP_OK) {
        return false;
    }

    // A stored device is valid unless its address is all zeros
    for (int i = 0; i < ESP_BD_ADDR_LEN; i++) {
        if (paired_device_cache.addr[i] != 0) {
            paired_device_cache.valid = true;
            break;
        }
    }
    return paired_device_cache.valid;
}

// Reconnection task
//
// Connects straight to the paired phone by its address. A phone answers a
// connection request from a device it is paired with whenever its Bluetooth
// is on; it answers an inquiry only while its Bluetooth settings are open,
// so the phone must not be searched for.
//
// A phone that was disconnected by its user is left alone, for as long as it
// stays within reach. A phone that is switched off, restarted or carried
// away reports the same reason as one whose user tapped disconnect, so while
// held off the phone is asked for its name: that reaches it without
// connecting, and the first time it does not answer the hold is lifted.
//
// A link that is still up just after the session closed is waited on,
// because the reason it closes is what tells a deliberate disconnect from a
// lost one.
static void bt_reconnect_task(void *pvParameters)
{
    TickType_t attempt_started = 0;
    TickType_t away_since = xTaskGetTickCount();

    while (1) {
        if (ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED)) {
            is_connecting = false;
            away_since = xTaskGetTickCount();
            vTaskDelay(pdMS_TO_TICKS(BT_CONNECTED_POLL_MS));
            continue;
        }

        if (held_off) {
            if (bt_load_paired_device()) {
                esp_bt_gap_read_remote_name(paired_device_cache.addr);
            }
            // The quick retries start from when the hold is lifted
            away_since = xTaskGetTickCount();
            vTaskDelay(pdMS_TO_TICKS(BT_HELD_PROBE_INTERVAL_MS));
            continue;
        }

        // An attempt whose result never arrived must not block the next one
        if (is_connecting &&
            (xTaskGetTickCount() - attempt_started) > pdMS_TO_TICKS(BT_CONNECT_ATTEMPT_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "No result from the last connection attempt, trying again");
            is_connecting = false;
        }

        bool link_closing = acl_up &&
            (xTaskGetTickCount() - session_closed_at) < pdMS_TO_TICKS(BT_LINK_CLOSE_WAIT_MS);

        if (!link_closing && !is_connecting) {
            if (bt_load_paired_device()) {
                ESP_LOGI(TAG, "Connecting to paired device %s", paired_device_cache.name);
                is_connecting = true;
                attempt_started = xTaskGetTickCount();
                esp_err_t ret = esp_hf_client_connect(paired_device_cache.addr);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to start connection: %s", esp_err_to_name(ret));
                    is_connecting = false;
                }
            } else {
                ESP_LOGI(TAG, "No paired device found, waiting for new connection");
            }
        }

        // Retry quickly at first, which covers a phone restarting or stepping
        // out; then slowly, so paging an absent phone does not keep taking
        // the radio from WiFi
        bool recently_lost = (xTaskGetTickCount() - away_since) < pdMS_TO_TICKS(BT_RECONNECT_FAST_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(recently_lost ? BT_RECONNECT_FAST_INTERVAL_MS : BT_RECONNECT_SLOW_INTERVAL_MS));
    }
}

// The paired phone's name, for messages
static const char *bt_phone_label(void)
{
    return (bt_load_paired_device() && paired_device_cache.name[0] != '\0') ? paired_device_cache.name : "The phone";
}

// The paired phone's name as an extra event field, with room left for more
static void bt_phone_extra(char *extra, size_t extra_len)
{
    char name[64];
    event_log_escape(bt_phone_label(), name, sizeof(name));
    snprintf(extra, extra_len, "\"phone\":\"%s\"", name);
}

// The HFP callback reports how each connection attempt ended
static void bt_connection_event_cb(event_type_t event, void *user_data)
{
    is_connecting = false;
    if (event & BT_EVENT_CONNECTED) {
        session_was_up = true;
        held_off = false;
    } else {
        session_closed_at = xTaskGetTickCount();
    }
}

// GAP callback for connection management
void bt_connection_manager_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
            if (param->acl_conn_cmpl_stat.stat == ESP_BT_STATUS_SUCCESS) {
                acl_up = true;
            }
            break;

        case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT: {
            int reason = param->acl_disconn_cmpl_stat.reason;
            ESP_LOGI(TAG, "Link closed, reason 0x%x", reason);
            // Only the end of an established session counts: a refused or
            // collided connection attempt must not stop the retries
            if (session_was_up) {
                char extra[112];
                bt_phone_extra(extra, sizeof(extra) - 16);
                snprintf(extra + strlen(extra), sizeof(extra) - strlen(extra), ",\"reason\":%d", reason);
                if (reason == ESP_BT_STATUS_HCI_PEER_USER) {
                    held_off = true;
                    event_log_with(EVENT_LOG_INFO, "bt.reconnect_held", extra,
                                   "%s ended the connection; not reconnecting while it stays in reach",
                                   bt_phone_label());
                } else {
                    event_log_with(EVENT_LOG_INFO, "bt.link_lost", extra,
                                   "Lost the link to %s; reconnecting", bt_phone_label());
                }
            }
            session_was_up = false;
            acl_up = false;
            break;
        }

        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Authentication success with device: %s", param->auth_cmpl.device_name);
                // Convert device name to string and store paired device info
                char device_name[32] = {0};
                strncpy(device_name, (const char*)param->auth_cmpl.device_name, sizeof(device_name) - 1);
                app_hf_store_paired_device(param->auth_cmpl.bda, device_name);
                ESP_LOGI(TAG, "Stored paired device: %s", device_name);
                // Invalidate cache to force refresh on next reconnection attempt
                paired_device_cache.valid = false;
            } else {
                ESP_LOGE(TAG, "Authentication failed: %d", param->auth_cmpl.stat);
                is_connecting = false;
            }
            break;

        case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
            // While held off, no answer means the phone has gone away
            if (held_off && param->read_rmt_name.stat != ESP_BT_STATUS_SUCCESS) {
                held_off = false;
                char extra[96];
                bt_phone_extra(extra, sizeof(extra));
                event_log_with(EVENT_LOG_INFO, "bt.reconnect_resumed", extra,
                               "%s went out of reach; reconnecting when it returns", bt_phone_label());
            }
            // Asked for when a phone connects whose name was not stored
            if (param->read_rmt_name.stat == ESP_BT_STATUS_SUCCESS && param->read_rmt_name.rmt_name[0] != '\0') {
                char device_name[32] = {0};
                strncpy(device_name, (const char*)param->read_rmt_name.rmt_name, sizeof(device_name) - 1);
                ESP_LOGI(TAG, "Remote device name: %s", device_name);
                ma_bell_state_set_bt_device_name(device_name);
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

    esp_err_t err = event_subscribe(BT_EVENT_CONNECTED | BT_EVENT_DISCONNECTED, bt_connection_event_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to subscribe to connection events: %s", esp_err_to_name(err));
        return err;
    }

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
