/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "nvs_flash.h"
#include "bt_app_hf.h"
#include "bt_app_core.h"
#include "app_hf_msg_set.h"
#include "ma_bell_state.h"
#include "app/events/event_system.h"
#include "audio/audio_bridge.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/ringbuf.h"
#include "time.h"
#include "sys/time.h"
#include "sdkconfig.h"

const char *c_hf_evt_str[] = {
    "CONNECTION_STATE_EVT",              /*!< connection state changed event */
    "AUDIO_STATE_EVT",                   /*!< audio connection state change event */
    "VR_STATE_CHANGE_EVT",                /*!< voice recognition state changed */
    "CALL_IND_EVT",                      /*!< call indication event */
    "CALL_SETUP_IND_EVT",                /*!< call setup indication event */
    "CALL_HELD_IND_EVT",                 /*!< call held indicator event */
    "NETWORK_STATE_EVT",                 /*!< network state change event */
    "SIGNAL_STRENGTH_IND_EVT",           /*!< signal strength indication event */
    "ROAMING_STATUS_IND_EVT",            /*!< roaming status indication event */
    "BATTERY_LEVEL_IND_EVT",             /*!< battery level indication event */
    "CURRENT_OPERATOR_EVT",              /*!< current operator name event */
    "RESP_AND_HOLD_EVT",                 /*!< response and hold event */
    "CLIP_EVT",                          /*!< Calling Line Identification notification event */
    "CALL_WAITING_EVT",                  /*!< call waiting notification */
    "CLCC_EVT",                          /*!< listing current calls event */
    "VOLUME_CONTROL_EVT",                /*!< audio volume control event */
    "AT_RESPONSE",                       /*!< audio volume control event */
    "SUBSCRIBER_INFO_EVT",               /*!< subscriber information event */
    "INBAND_RING_TONE_EVT",              /*!< in-band ring tone settings */
    "LAST_VOICE_TAG_NUMBER_EVT",         /*!< requested number from AG event */
    "RING_IND_EVT",                      /*!< ring indication event */
    "PKT_STAT_EVT",                      /*!< requested number of packet status event */
};

// esp_hf_client_connection_state_t
const char *c_connection_state_str[] = {
    "disconnected",
    "connecting",
    "connected",
    "slc_connected",
    "disconnecting",
};

// esp_hf_client_audio_state_t
const char *c_audio_state_str[] = {
    "disconnected",
    "connecting",
    "connected",
    "connected_msbc",
};

/// esp_hf_vr_state_t
const char *c_vr_state_str[] = {
    "disabled",
    "enabled",
};

// esp_hf_service_availability_status_t
const char *c_service_availability_status_str[] = {
    "unavailable",
    "available",
};

// esp_hf_roaming_status_t
const char *c_roaming_status_str[] = {
    "inactive",
    "active",
};

// esp_hf_client_call_state_t
const char *c_call_str[] = {
    "NO call in progress",
    "call in progress",
};

// esp_hf_client_callsetup_t
const char *c_call_setup_str[] = {
    "NONE",
    "INCOMING",
    "OUTGOING_DIALING",
    "OUTGOING_ALERTING"
};

// esp_hf_client_callheld_t
const char *c_call_held_str[] = {
    "NONE held",
    "Held and Active",
    "Held",
};

// esp_hf_response_and_hold_status_t
const char *c_resp_and_hold_str[] = {
    "HELD",
    "HELD ACCEPTED",
    "HELD REJECTED",
};

// esp_hf_client_call_direction_t
const char *c_call_dir_str[] = {
    "outgoing",
    "incoming",
};

// esp_hf_client_call_state_t
const char *c_call_state_str[] = {
    "active",
    "held",
    "dialing",
    "alerting",
    "incoming",
    "waiting",
    "held_by_resp_hold",
};

// esp_hf_current_call_mpty_type_t
const char *c_call_mpty_type_str[] = {
    "single",
    "multi",
};

// esp_hf_volume_control_target_t
const char *c_volume_control_target_str[] = {
    "SPEAKER",
    "MICROPHONE"
};

// esp_hf_at_response_code_t
const char *c_at_response_code_str[] = {
    "OK",
    "ERROR"
    "ERR_NO_CARRIER",
    "ERR_BUSY",
    "ERR_NO_ANSWER",
    "ERR_DELAYED",
    "ERR_BLACKLILSTED",
    "ERR_CME",
};

// esp_hf_subscriber_service_type_t
const char *c_subscriber_service_type_str[] = {
    "unknown",
    "voice",
    "fax",
};

// esp_hf_client_in_band_ring_state_t
const char *c_inband_ring_state_str[] = {
    "NOT provided",
    "Provided",
};

extern esp_bd_addr_t peer_addr;
// If you want to connect a specific device, add it's address here
// esp_bd_addr_t peer_addr = {0xac, 0x67, 0xb2, 0x53, 0x77, 0xbe};

#if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI

// Note: Ring buffers are managed by audio_bridge module
// Audio bridge start/stop is called from AUDIO_STATE_EVT handler
// bt_rx_ringbuf: Bluetooth → Phone (incoming audio)
// bt_tx_ringbuf: Phone → Bluetooth (outgoing audio)

static uint32_t bt_app_hf_client_outgoing_cb(uint8_t *p_buf, uint32_t sz)
{
    // Get ring buffer from audio bridge (Phone → Bluetooth)
    RingbufHandle_t bt_tx_ringbuf = audio_bridge_get_bt_tx_ringbuf();
    if (!bt_tx_ringbuf) {
        return 0;
    }

    size_t item_size = 0;
    uint8_t *data = xRingbufferReceiveUpTo(bt_tx_ringbuf, &item_size, 0, sz);
    if (item_size == sz) {
        memcpy(p_buf, data, item_size);
        vRingbufferReturnItem(bt_tx_ringbuf, data);
        return sz;
    } else if (0 < item_size) {
        vRingbufferReturnItem(bt_tx_ringbuf, data);
        return 0;
    } else {
        // data not enough, do not read
        return 0;
    }
}

static void bt_app_hf_client_incoming_cb(const uint8_t *buf, uint32_t sz)
{
    // Get ring buffer from audio bridge (Bluetooth → Phone)
    RingbufHandle_t bt_rx_ringbuf = audio_bridge_get_bt_rx_ringbuf();
    if (!bt_rx_ringbuf) {
        return;
    }

    BaseType_t done = xRingbufferSend(bt_rx_ringbuf, (uint8_t *)buf, sz, 0);
    if (!done) {
        ESP_LOGE(BT_HF_TAG, "BT RX ring buffer send fail");
    }
    // Note: We don't call esp_hf_client_outgoing_data_ready() here anymore
    // because outgoing data comes from the phone microphone (I2S RX),
    // not from loopback of incoming Bluetooth audio
}
void bt_app_hf_register_data_callbacks(void)
{
    ESP_LOGI(BT_HF_TAG, "Registering HFP audio data callbacks (HCI path)");
    esp_hf_client_register_data_callback(bt_app_hf_client_incoming_cb,
                                          bt_app_hf_client_outgoing_cb);
}

#else /* PCM path - no software audio handling */

void bt_app_hf_register_data_callbacks(void)
{
    ESP_LOGI(BT_HF_TAG, "PCM audio path - no data callbacks needed");
}

#endif /* #if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI */

static const char *TAG = "bt_app_hf";

/* callback for HF_CLIENT */
void bt_app_hf_client_cb(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param)
{
    switch (event) {
        case ESP_HF_CLIENT_CONNECTION_STATE_EVT:
            ESP_LOGI(TAG, "Connection state: %d", param->conn_stat.state);
            if (param->conn_stat.state == ESP_HF_CLIENT_CONNECTION_STATE_CONNECTED) {
                // Update state
                ma_bell_state_update_bluetooth_bits(BT_STATE_CONNECTED, 0);
                // Publish connection event
                event_publish(BT_EVENT_CONNECTED, NULL);
            } else if (param->conn_stat.state == ESP_HF_CLIENT_CONNECTION_STATE_DISCONNECTED) {
                // Update state
                ma_bell_state_update_bluetooth_bits(0, BT_STATE_CONNECTED);
                // Publish disconnection event
                event_publish(BT_EVENT_DISCONNECTED, NULL);
            }
            break;

        case ESP_HF_CLIENT_AUDIO_STATE_EVT:
            ESP_LOGI(TAG, "Audio state: %s", c_audio_state_str[param->audio_stat.state]);
            if (param->audio_stat.state == ESP_HF_CLIENT_AUDIO_STATE_CONNECTED) {
                // Audio connected - start audio bridge tasks
                ma_bell_state_update_bluetooth_bits(BT_STATE_AUDIO_CONNECTED, 0);
#if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI
                audio_bridge_start();
#endif
                event_publish(BT_EVENT_AUDIO_CONNECTED, NULL);
            } else if (param->audio_stat.state == ESP_HF_CLIENT_AUDIO_STATE_DISCONNECTED) {
                // Audio disconnected - stop audio bridge tasks
#if CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI
                audio_bridge_stop();
#endif
                ma_bell_state_update_bluetooth_bits(0, BT_STATE_AUDIO_CONNECTED);
                event_publish(BT_EVENT_AUDIO_DISCONNECTED, NULL);
            }
            break;

        case ESP_HF_CLIENT_BVRA_EVT:
            ESP_LOGI(TAG, "VR state: %d", param->bvra.value);
            break;

        case ESP_HF_CLIENT_RING_IND_EVT:
            ESP_LOGI(TAG, "Incoming call ring indication");
            // Update phone state to indicate ringing
            ma_bell_state_update_phone_bits(PHONE_STATE_RINGING, 0);
            break;

        case ESP_HF_CLIENT_CIND_CALL_EVT:
            ESP_LOGI(TAG, "Call state changed: %s", c_call_str[param->call.status]);
            if (param->call.status == 0) {  // No call in progress
                // Call ended (hung up)
                ESP_LOGI(TAG, "Call ended - returning to idle state");
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING | PHONE_STATE_OFF_HOOK);
                ma_bell_state_update_bluetooth_bits(0, BT_STATE_IN_CALL | BT_STATE_AUDIO_CONNECTED);
                // Publish call ended event
                event_publish(BT_EVENT_CALL_ENDED, NULL);
            } else if (param->call.status == 1) {  // Call in progress
                // Call active
                ESP_LOGI(TAG, "Call active - updating state");
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
                ma_bell_state_update_bluetooth_bits(BT_STATE_IN_CALL, 0);
                // Publish call started event
                event_publish(BT_EVENT_CALL_STARTED, NULL);
            }
            break;

        case ESP_HF_CLIENT_CIND_CALL_SETUP_EVT:
            ESP_LOGI(TAG, "Call setup state: %s", c_call_setup_str[param->call_setup.status]);
            if (param->call_setup.status == ESP_HF_CALL_SETUP_STATUS_INCOMING) {
                // Incoming call
                ESP_LOGI(TAG, "Incoming call detected");
                ma_bell_state_update_phone_bits(PHONE_STATE_RINGING, 0);
            } else if (param->call_setup.status == ESP_HF_CALL_SETUP_STATUS_IDLE) {
                // Call setup ended (could be hangup, reject, or timeout)
                ESP_LOGI(TAG, "Call setup ended - clearing ringing state");
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
            }
            break;

        case ESP_HF_CLIENT_CIND_CALL_HELD_EVT:
            ESP_LOGI(TAG, "Call held state: %s", c_call_held_str[param->call_held.status]);
            if (param->call_held.status == ESP_HF_CALL_HELD_STATUS_NONE) {
                // Call released
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
                ma_bell_state_update_bluetooth_bits(0, BT_STATE_IN_CALL);
            } else if (param->call_held.status == ESP_HF_CALL_HELD_STATUS_HELD_AND_ACTIVE) {
                // Call held and active
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
                ma_bell_state_update_bluetooth_bits(BT_STATE_IN_CALL, 0);
            } else if (param->call_held.status == ESP_HF_CALL_HELD_STATUS_HELD) {
                // Call held
                ma_bell_state_update_phone_bits(0, PHONE_STATE_RINGING);
                ma_bell_state_update_bluetooth_bits(BT_STATE_IN_CALL, 0);
            }
            break;

        case ESP_HF_CLIENT_CIND_SERVICE_AVAILABILITY_EVT:
        case ESP_HF_CLIENT_CIND_SIGNAL_STRENGTH_EVT:
        case ESP_HF_CLIENT_CIND_ROAMING_STATUS_EVT:
        case ESP_HF_CLIENT_CIND_BATTERY_LEVEL_EVT:
        case ESP_HF_CLIENT_COPS_CURRENT_OPERATOR_EVT:
        case ESP_HF_CLIENT_BTRH_EVT:
        case ESP_HF_CLIENT_CLIP_EVT:
        case ESP_HF_CLIENT_CCWA_EVT:
        case ESP_HF_CLIENT_CLCC_EVT:
        case ESP_HF_CLIENT_VOLUME_CONTROL_EVT:
        case ESP_HF_CLIENT_AT_RESPONSE_EVT:
        case ESP_HF_CLIENT_CNUM_EVT:
        case ESP_HF_CLIENT_BSIR_EVT:
        case ESP_HF_CLIENT_BINP_EVT:
        case ESP_HF_CLIENT_PKT_STAT_NUMS_GET_EVT:
            // These events are not currently handled
            ESP_LOGD(TAG, "Unhandled event: %d", event);
            break;

        default:
            ESP_LOGW(TAG, "Unknown event: %d", event);
            break;
    }
}
